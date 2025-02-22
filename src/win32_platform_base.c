#include <windows.h>
#include "base_types.h"
#include "platform.h"

/* internal void */
/* RenderWeirdGradient(GameOffscreenBuffer Buffer, u64 BlueOffset, u64 GreenOffset) */
/* { */
/*     u8 *Row = (u8 *)Buffer.Memory; */    
/*     for(int Y = 0; */
/*         Y < Buffer.Height; */
/*         ++Y) */
/*     { */
/*         u32 *Pixel = (u32 *)Row; */
/*         for(int X = 0; */
/*             X < Buffer.Width; */
/*             ++X) */
/*         { */
/*             u8 Blue = (X + BlueOffset); */
/*             u8 Green = (Y + GreenOffset); */

/*             *Pixel++ = ((Green << 8) | Blue); */
/*         } */
        
/*         Row += Buffer.Pitch; */
/*     } */
/* } */

internal u8
debug_platform_write_entire_file(char *Filename, u64 MemorySize, void *Memory)
{
    u8 Result = 0;

    HANDLE FileHandle = CreateFile(Filename, GENERIC_WRITE, 0, 0, CREATE_ALWAYS, 0, 0);
    /* DWORD LastError = GetLastError(); */

    if(FileHandle != INVALID_HANDLE_VALUE)
    {
        LPDWORD BytesWritten = 0;
        if(WriteFile(FileHandle, Memory, MemorySize, BytesWritten, 0))
        { 
            if (MemorySize == *BytesWritten)
            {
                // file read success
                Result = (*BytesWritten == MemorySize);
            }
        }
        else
        {
            // logging
            /* DWORD LastError = GetLastError(); */
        }

        CloseHandle(FileHandle);

    }
    else
    {
        // logging
    }

    return Result;
}

internal void
debug_platform_free_file_memory(void *Memory, u64 MemorySize)
{
    // win32 free file memory
    if(Memory && VirtualFree(Memory, MemorySize, MEM_DECOMMIT|MEM_RELEASE))
    {
        // freed
    }
    else 
    {
        // unable to free
    }
}

internal File
debug_platform_read_entire_file(char *Filename)
{
    File Result = {0};

    HANDLE FileHandle = CreateFile(Filename, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);
    // DWORD LastError = GetLastError();

    if(FileHandle != INVALID_HANDLE_VALUE)
    {
        LARGE_INTEGER FileSize;
        if(GetFileSizeEx(FileHandle, &FileSize)) {
            Assert(FileSize.QuadPart <= max_u64);
            u64 FileSize64 = FileSize.QuadPart;
            void *Buffer = VirtualAlloc(0, FileSize.QuadPart, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
            if(Buffer)
            {
                LPDWORD BytesRead = 0;
                // C is required to execute these in order
                // A && B && C
                // will always be A THEN B THEN C
                if(ReadFile(FileHandle, Buffer, FileSize.QuadPart, BytesRead, 0) &&
                        // if we got the file size
                        // someone truncated the file
                        // we read the file
                        // and we didn't read back the expected size we know something went wrong
                        (FileSize64 == *BytesRead))
                {
                    // file read success
                    Result.Length = *BytesRead;
                    Result.Data = Buffer;
                }
                else
                {
                    // logging
                    debug_platform_free_file_memory(Result.Data, FileSize64);
                }
            }
            else
            {
                // logging
            }
        }
        else
        {
            // logging
        }

        CloseHandle(FileHandle);
    }
    else
    {
        // logging
    }

    return Result;
}

internal s32
round_f32_to_s32(f32 Real32)
{
    s32 Result = (s32)(Real32 + 0.5f);
    // TODO(casey): Intrinsic????
    return(Result);
}

internal u32
round_f32_to_u32(f32 Real32)
{
    u32 Result = (u32)(Real32 + 0.5f);
    // TODO(casey): Intrinsic????
    return(Result);
}

internal void
draw_rectangle(GameOffscreenBuffer *buffer,
              f32 real_min_x, f32 real_min_y, f32 real_max_x, f32 real_max_y,
              f32 R, f32 G, f32 B)
{
    // TODO(casey): Floating point color tomorrow!!!!!!

    s32 MinX = round_f32_to_s32(real_min_x);
    s32 MinY = round_f32_to_s32(real_min_y);
    s32 MaxX = round_f32_to_s32(real_max_x);
    s32 MaxY = round_f32_to_s32(real_max_y);

    if(MinX < 0)
    {
        MinX = 0;
    }

    if(MinY < 0)
    {
        MinY = 0;
    }

    if(MaxX > buffer->Width)
    {
        MaxX = buffer->Width;
    }

    if(MaxY > buffer->Height)
    {
        MaxY = buffer->Height;
    }
    
    // 0x AA RR GG BB
    u32 Color = ((round_f32_to_u32(R * 255.0f) << 16) |
                    (round_f32_to_u32(G * 255.0f) << 8) |
                    (round_f32_to_u32(B * 255.0f) << 0));

    u8 *Row = ((u8 *)buffer->Memory +
                  MinX*buffer->BytesPerPixel +
                  MinY*buffer->Pitch);
    for(int Y = MinY;
        Y < MaxY;
        ++Y)
    {
        u32 *Pixel = (u32 *)Row;
        for(int X = MinX;
            X < MaxX;
            ++X)
        {            
            *Pixel++ = Color;
        }
        
        Row += buffer->Pitch;
    }
}

internal u32
truncate_f32_to_u32(f32 f)
{
    u32 Result = (u32)f;
    return(Result);
}

typedef struct TileMap
{
    u32 count_x;
    u32 count_y;
    f32 upper_left_x;
    f32 upper_left_y;
    f32 tile_width;
    f32 tile_height;

    u32 *tiles;
} TileMap;

inline u32
get_tile_value_unchecked(TileMap *tile_map, u32 tile_x, u32 tile_y)
{
    u32 tile_map_value = tile_map->tiles[((tile_y*tile_map->count_x) + tile_x)];
    return tile_map_value;
}

internal u32
is_tile_map_point_empty(TileMap *tile_map, f32 test_x, f32 test_y)
{
    u32 empty = FALSE;

    f32 x = ((test_x - tile_map->upper_left_x)/tile_map->tile_width);
    f32 y = ((test_y - tile_map->upper_left_y)/tile_map->tile_height);

    u32 player_tile_x = truncate_f32_to_u32(x);
    u32 player_tile_y = truncate_f32_to_u32(y);

    if((player_tile_x >= 0) && (player_tile_x < tile_map->count_x) &&
            (player_tile_y >= 0) && (player_tile_y < tile_map->count_y))
    {

        /* u32 tile_map_value = TileMap[player_tile_y][player_tile_x]; */

        // accessing a 1D thing in a 2D way
        // Array[y*w + x]
        //
        u32 tile_map_value = get_tile_value_unchecked(tile_map, player_tile_x, player_tile_y);
        empty = (tile_map_value == 0);
    }

    return empty;
}

typedef struct World
{
    u32 tile_map_count_x;
    u32 tile_map_count_y;

    TileMap *tile_maps;
} World;

inline TileMap*
get_tile_map(World *world, u32 tile_map_x, u32 tile_map_y)
{
    TileMap *tile_map = 0;
    if((tile_map_x >= 0) && (tile_map_x < world->tile_map_count_x) &&
        (tile_map_y >= 0) && (tile_map_y < world->tile_map_count_y))
    {
         tile_map = &world->tile_maps[((tile_map_y*world->tile_map_count_x) + tile_map_x)];
    }

    return tile_map;
}

internal u32
is_world_point_empty(World *world, u32 tile_map_x, u32 tile_map_y, f32 test_x, f32 test_y)
{
    u32 empty = FALSE;

    TileMap *tile_map = get_tile_map(world, tile_map_x, tile_map_y); 

    if (tile_map)
    {
        f32 x = ((test_x - tile_map->upper_left_x)/tile_map->tile_width);
        f32 y = ((test_y - tile_map->upper_left_y)/tile_map->tile_height);

        u32 player_tile_x = truncate_f32_to_u32(x);
        u32 player_tile_y = truncate_f32_to_u32(y);

        // where we are in "tilemap space"

        if((player_tile_x >= 0) && (player_tile_x < tile_map->count_x) &&
                (player_tile_y >= 0) && (player_tile_x < tile_map->count_y))
        {
            u32 tile_map_value = get_tile_value_unchecked(tile_map, player_tile_x, player_tile_y);
            empty = (tile_map_value == 0);
        }

    }
    return empty;
}
internal void
game_update_and_render(GameMemory *GameMemory, GameInput *game_input, GameOffscreenBuffer *buffer)
{
    GameState *game_state = (GameState *)GameMemory->PermenantStorage;
    Assert(sizeof(game_state) <= GameMemory->PermenantStorageSize);
    if(!GameMemory->IsInitialized) 
    {
        // The problem with this layout is that we're streaming from a HD
        // and _any_ read could fail at some point. For instance, if you're reading
        // from a CD and the user removes the CD from the drive the stream would break.
        // so that's a failure case for _every_ read.
        //
        // Threading safety problems here also since each thread would need its own
        // file handle.
        //
        // This requires that calls be sync'd and it stalls the whole program because
        // we're tying the program to slow HD reads.
        //
        /* char *Filename = "test.bmp"; */
        /* file_handle *File = OpenFile(Filename); */
        /* u8 Buffer[128]; */
        /* if (Read(file, sizeof(Buffer), Buffer)) */
        /* { */
        /*     // Use buffer */
        /* } */
        /* else */ 
        /* { */
        /*     // error */     
        /* } */
        /* CloseFile(File); */

        // TODO: this is broken
        /* char *Filename = "../assets/images/test.png"; */
        /* File Memory = debug_platform_read_entire_file(Filename); */
        /* if(Memory.Length > 0) */
        /* { */
        /*     debug_platform_write_entire_file("../assets/foo.png", Memory.Length, Memory.Data); */
        /*     debug_platform_free_file_memory(Memory.Data, Memory.Length); */
        /* } */

        /* GameState->GreenOffset = 0; */
        /* GameState->BlueOffset = 0; */
        
        // maybe this should be done on the os side of things
        GameMemory->IsInitialized = 1;

        // starting position for the player character
        game_state->player_x = 200.0f;
        game_state->player_y = 100.0f;
    }

    // take the window height and window width to determine how many columns and rows
    //
    // ex:
    // 540 h / 60 = 8 rows
    // 960 w / 60 = 16 columns
    //
    // then loop through all of them and if hte tile is a 1 then we a section of the
    // level otherwise, we draw part of the wall / background
    #define TILE_MAP_COUNT_Y 9 
    #define TILE_MAP_COUNT_X 17
    u32 Tiles00[TILE_MAP_COUNT_Y][TILE_MAP_COUNT_X] =
    {
        { 1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1, 1},
        { 1, 1, 0, 0,  0, 1, 0, 0,  0, 0, 0, 0,  0, 1, 0, 1, 1},
        { 1, 1, 0, 0,  0, 0, 0, 0,  1, 0, 0, 0,  0, 0, 1, 1, 1},
        { 1, 0, 0, 0,  0, 0, 0, 0,  1, 0, 0, 0,  0, 0, 0, 1, 1},
        { 1, 0, 0, 0,  0, 1, 0, 0,  1, 0, 0, 0,  0, 0, 0, 0, 0},
        { 1, 1, 0, 0,  0, 1, 0, 0,  1, 0, 0, 0,  0, 1, 0, 1, 1},
        { 1, 0, 0, 0,  0, 1, 0, 0,  1, 0, 0, 0,  1, 0, 0, 1, 1},
        { 1, 1, 1, 1,  1, 0, 0, 0,  0, 0, 0, 0,  0, 1, 0, 1, 1},
        { 1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1, 1}
    };
    u32 Tiles01[TILE_MAP_COUNT_Y][TILE_MAP_COUNT_X] =
    {
        { 1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1, 1},
        { 1, 1, 0, 0,  0, 1, 0, 0,  0, 0, 0, 0,  0, 1, 0, 0, 1},
        { 1, 1, 0, 0,  0, 0, 0, 0,  1, 0, 0, 0,  0, 0, 1, 0, 1},
        { 1, 0, 0, 0,  0, 0, 0, 0,  1, 0, 0, 0,  0, 0, 0, 0, 1},
        { 1, 0, 0, 0,  0, 1, 0, 0,  1, 0, 0, 0,  0, 0, 0, 0, 0},
        { 1, 1, 0, 0,  0, 1, 0, 0,  1, 0, 0, 0,  0, 1, 0, 0, 1},
        { 1, 0, 0, 0,  0, 1, 0, 0,  1, 0, 0, 0,  1, 0, 0, 0, 1},
        { 1, 1, 1, 1,  1, 0, 0, 0,  0, 0, 0, 0,  0, 1, 0, 0, 1},
        { 1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1, 1}
    };
    u32 Tiles10[TILE_MAP_COUNT_Y][TILE_MAP_COUNT_X] =
    {
        { 1, 1, 1, 1,  1, 1, 1, 1,  0, 1, 1, 1,  1, 1, 1, 1, 1},
        { 1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 1},
        { 1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 1},
        { 1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 1},
        { 0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 1},
        { 1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 1},
        { 1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 1},
        { 1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 1},
        { 1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1, 1}
    };
    u32 Tiles11[TILE_MAP_COUNT_Y][TILE_MAP_COUNT_X] =
    {
        { 1, 1, 1, 1,  1, 1, 1, 1,  0, 1, 1, 1,  1, 1, 1, 1, 1},
        { 1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 1},
        { 1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 1},
        { 1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 1},
        { 0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 1},
        { 1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 1},
        { 1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 1},
        { 1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 1},
        { 1, 1, 1, 1,  1, 1, 1, 1,  0, 1, 1, 1,  1, 1, 1, 1, 1}
    };
    TileMap tile_map0 = {
        .count_x = TILE_MAP_COUNT_X,
        .count_y = TILE_MAP_COUNT_Y,
        .upper_left_x = 0,
        .upper_left_y = -30,
        .tile_width = 60,
        .tile_height = 60,
        .tiles = (u32 *)Tiles00,
    };
    TileMap tile_map1 = {
        .count_x = TILE_MAP_COUNT_X,
        .count_y = TILE_MAP_COUNT_Y,
        .upper_left_x = 0,
        .upper_left_y = -30,
        .tile_width = 60,
        .tile_height = 60,
        .tiles = (u32 *)Tiles01,
    };
    TileMap tile_map2 = {
        .count_x = TILE_MAP_COUNT_X,
        .count_y = TILE_MAP_COUNT_Y,
        .upper_left_x = 0,
        .upper_left_y = -30,
        .tile_width = 60,
        .tile_height = 60,
        .tiles = (u32 *)Tiles10,
    };
    TileMap tile_map3 = {
        .count_x = TILE_MAP_COUNT_X,
        .count_y = TILE_MAP_COUNT_Y,
        .upper_left_x = 0,
        .upper_left_y = -30,
        .tile_width = 60,
        .tile_height = 60,
        .tiles = (u32 *)Tiles11,
    };

    TileMap tile_maps[2][2] = {
        {tile_map0, tile_map1},
        {tile_map2, tile_map3}
    };

    World world = {
        .tile_map_count_x = 2,
        .tile_map_count_y = 2,
        .tile_maps = (TileMap *)tile_maps
    };

    TileMap *tile_map = &tile_maps[0][0];

    f32 PlayerR = 1.0f;
    f32 PlayerG = 1.0f;
    f32 PlayerB = 0.0f;

    f32 PlayerWidth = 0.5f*tile_map->tile_width;
    f32 PlayerHeight = 0.5f*tile_map->tile_height;

    GameInput *Input = game_input;
    // NOTE(casey): Use digital movement tuning
    f32 dPlayerX = 0.0f; // pixels/second
    f32 dPlayerY = 0.0f; // pixels/second
                         //
    if(Input->Controller.Up.EndedDown) {
        dPlayerY = -1.0f;
    }

    if(Input->Controller.Down.EndedDown) {
        dPlayerY = 1.0f;
    }

    if(Input->Controller.Left.EndedDown) {
        dPlayerX = -1.0f;
    }

    if(Input->Controller.Right.EndedDown) {
        dPlayerX = 1.0f;
    }

    dPlayerX *= 256.0f;
    dPlayerY *= 256.0f;

    f32 new_player_x = game_state->player_x + Input->dt_for_frame*dPlayerX;
    f32 new_player_y = game_state->player_y + Input->dt_for_frame*dPlayerY;

    u8 a = is_tile_map_point_empty(tile_map, (new_player_x - 0.5f*PlayerWidth), new_player_y);
    u8 b = is_tile_map_point_empty(tile_map, (new_player_x + 0.5f*PlayerWidth), new_player_y);
    u8 c = is_tile_map_point_empty(tile_map, new_player_x, (new_player_y - PlayerHeight));
    u8 d = is_tile_map_point_empty(tile_map, new_player_x, new_player_y);

    u8 no_collision =  a && b && c && d;
    if (no_collision)
    {
        game_state->player_x = new_player_x;
        game_state->player_y = new_player_y;
    }

    draw_rectangle(buffer, 0.0f, 0.0f, (f32)buffer->Width, (f32)buffer->Height,
                  0.0f, 0.0f, 0.0f);

    for(int Row = 0;
        Row < 9;
        ++Row)
    {
        for(int Column = 0;
            Column < 17;
            ++Column)
        {
            u32 TileID = get_tile_value_unchecked(tile_map, Column, Row);
            f32 Gray = 0.5f;
            if(TileID == 1)
            {
                Gray = 0.0f;
            }

            f32 MinX = tile_map->upper_left_x + ((f32)Column)*tile_map->tile_width;
            f32 MinY = tile_map->upper_left_y + ((f32)Row)*tile_map->tile_height;
            f32 MaxX = MinX + tile_map->tile_width;
            f32 MaxY = MinY + tile_map->tile_height;
            
#if 0
            char b[256];
            _snprintf_s(
                    b, 
                    sizeof(b), 
                    256,
                    "Row: %d, Column: %d\n", 
                    Row, 
                    Column);
            OutputDebugStringA(b);
#endif

            draw_rectangle(buffer, MinX, MinY, MaxX, MaxY, Gray, Gray, Gray);
        }
    }
    
    // draw the player
    f32 PlayerLeft = game_state->player_x - 0.5f*PlayerWidth;
    f32 PlayerTop = game_state->player_y - PlayerHeight;
    draw_rectangle(buffer,
                  PlayerLeft, PlayerTop,
                  PlayerLeft + PlayerWidth,
                  PlayerTop + PlayerHeight,
                  PlayerR, PlayerG, PlayerB);

    /* draw_rectangle(Buffer, 0.0f, 0.0f, (f32)Buffer->Width, (f32)Buffer->Height, 0.4f, .5f, 1.0f); */
    /* draw_rectangle(Buffer, 200.0f, 20.0f, 40.f, 40.f, 1.f, 0.f, 0.f); */
    /* RenderWeirdGradient(*Buffer, GameState->BlueOffset, GameState->GreenOffset); */
}
