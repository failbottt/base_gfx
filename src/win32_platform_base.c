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

#include "math.h"
inline u32 
floor_f32_to_u32(f32 f)
{
    u32 result = (u32)floorf(f);
    return(result);
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

typedef struct TileMap
{
    u32 *tiles;
} TileMap;

typedef struct World
{
    u32 count_x;
    u32 count_y;
    f32 upper_left_x;
    f32 upper_left_y;
    f32 tile_width;
    f32 tile_height;

    u32 tile_map_count_x;
    u32 tile_map_count_y;

    TileMap *tile_maps;
} World;

inline u32
get_tile_value_unchecked(World *world, TileMap *tile_map, u32 tile_x, u32 tile_y)
{
    Assert(tile_map);
    Assert((tile_x >= 0) && (tile_x < world->count_x) &&
            (tile_y >= 0) && (tile_y < world->count_y));

    u32 tile_map_value = tile_map->tiles[((tile_y*world->count_x) + tile_x)];
    return tile_map_value;
}


inline TileMap*
get_tile_map(World *world, u32 tile_map_x, u32 tile_map_y)
{
    TileMap *tile_map = 0;
    if((tile_map_x >= 0) && (tile_map_x < world->tile_map_count_x) &&
        (tile_map_y >= 0) && (tile_map_y < world->tile_map_count_y))
    {
        /*  accessing a 1D thing in a 2D way */
        /*  Array[y*w + x] */
        tile_map = &world->tile_maps[((tile_map_y*world->tile_map_count_x) + tile_map_x)];
    }

    return tile_map;
}

internal u32
is_tile_map_point_empty(World *world, TileMap *tile_map, f32 test_tile_x, f32 test_tile_y)
{
    u32 empty = FALSE;

    if (tile_map)
    {
        if((test_tile_x >= 0) && (test_tile_x < world->count_x) &&
                (test_tile_y >= 0) && (test_tile_y < world->count_y))
        {

            /* TileMap *tile_map = get_tile_map(tile_map, player_tile_x, player_tile_y); */
            /* u32 tile_map_value = TileMap[player_tile_y][player_tile_x]; */

            // accessing a 1D thing in a 2D way
            // Array[y*w + x]
            //
            u32 tile_map_value = get_tile_value_unchecked(world, tile_map, (u32)test_tile_x,(u32)test_tile_y);
            empty = (tile_map_value == 0);
        }
    }

    return empty;
}

typedef struct CanonicalPosition
{
    u32 tile_map_x;
    u32 tile_map_y; 

    u32 tile_x;
    u32 tile_y;

    f32 tile_rel_x; 
    f32 tile_rel_y; 
} CanonicalPosition;

// TODO(casey): Is this ever necessary?
typedef struct RawPosition
{
    u32 tile_map_x;
    u32 tile_map_y;

    // NOTE(casey): Tile-map relative X and Y
    f32 x;
    f32 y;
} RawPosition;

inline CanonicalPosition
get_canonical_world_position(World *world, RawPosition *raw_position)
{
    CanonicalPosition result = {};

    result.tile_map_x = raw_position->tile_map_x;
    result.tile_map_y = raw_position->tile_map_y;

    f32 x = raw_position->x - world->upper_left_x;
    f32 y = raw_position->y - world->upper_left_y;
    result.tile_x = floor_f32_to_u32(x / world->tile_width);
    result.tile_y = floor_f32_to_u32(y / world->tile_height);

    result.tile_rel_x = x - result.tile_x*world->tile_width;
    result.tile_rel_y = y - result.tile_y*world->tile_height;

    Assert(result.tile_rel_x >= 0);
    Assert(result.tile_rel_y >= 0);
    Assert(result.tile_rel_x < world->tile_width);
    Assert(result.tile_rel_y < world->tile_height);

    if (result.tile_x < 0)
    {
        result.tile_x = world->count_x + result.tile_x;
        --result.tile_map_x;
    }

    if (result.tile_y < 0)
    {
        result.tile_y = world->count_y + result.tile_y;
        --result.tile_map_y;
    }

    if (result.tile_x >= world->count_x)
    {
        result.tile_x = result.tile_x - world->count_x;
        ++result.tile_map_x;
    }

    if (raw_position->tile_map_y >= world->count_y)
    {
        result.tile_y =  result.tile_y - world->count_y;
        ++raw_position->tile_map_y;
    }

    return result;
}

inline u32
is_world_point_empty(World *world, RawPosition *raw_position)
{
    u32 empty = FALSE;

    CanonicalPosition can_pos = get_canonical_world_position(world, raw_position);
    TileMap *tile_map = get_tile_map(world, can_pos.tile_map_x, can_pos.tile_map_y);
    empty = is_tile_map_point_empty(world, tile_map, (f32)can_pos.tile_x, (f32)can_pos.tile_y);
    return empty;
}

internal void
game_update_and_render(GameMemory *GameMemory, GameInput *game_input, GameOffscreenBuffer *buffer)
{

    // take the window height and window width to determine how many columns and rows
    //
    // ex:
    // 540 h / 60 = 8 rows
    // 960 w / 60 = 16 columns
    //
    // then loop through all of them and if the tile is a 1 then we a section of the
    // level otherwise, we draw part of the wall / background
    #define TILE_MAP_COUNT_Y 9 
    #define TILE_MAP_COUNT_X 17
    u32 Tiles00[TILE_MAP_COUNT_Y][TILE_MAP_COUNT_X] =
    {
        {1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1, 1},
        {1, 1, 0, 0,  0, 1, 0, 0,  0, 0, 0, 0,  0, 1, 0, 1, 1},
        {1, 1, 0, 0,  0, 0, 0, 0,  1, 0, 0, 0,  0, 0, 1, 0, 1},
        {1, 0, 0, 0,  0, 0, 0, 0,  1, 0, 0, 0,  0, 0, 0, 0, 1},
        {1, 0, 0, 0,  0, 1, 0, 0,  1, 0, 0, 0,  0, 0, 0, 0, 1},
        {1, 1, 0, 0,  0, 1, 0, 0,  1, 0, 0, 0,  0, 1, 0, 0, 1},
        {1, 0, 0, 0,  0, 1, 0, 0,  1, 0, 0, 0,  1, 0, 0, 0, 1},
        {1, 1, 1, 1,  1, 0, 0, 0,  0, 0, 0, 0,  0, 1, 0, 0, 1},
        {1, 1, 1, 1,  1, 1, 1, 1,  0, 1, 1, 1,  1, 1, 1, 1, 1},
    };
    
    u32 Tiles01[TILE_MAP_COUNT_Y][TILE_MAP_COUNT_X] =
    {
        {1, 1, 1, 1,  1, 1, 1, 1,  0, 1, 1, 1,  1, 1, 1, 1, 1},
        {1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 1},
        {1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 1},
        {1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 1},
        {1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 0},
        {1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 1},
        {1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 1},
        {1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 1},
        {1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1, 1},
    };
    
    u32 Tiles10[TILE_MAP_COUNT_Y][TILE_MAP_COUNT_X] =
    {
        {1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1, 1},
        {1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 1},
        {1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 1},
        {1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 1},
        {0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 1},
        {1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 1},
        {1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 1},
        {1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 1},
        {1, 1, 1, 1,  1, 1, 1, 1,  0, 1, 1, 1,  1, 1, 1, 1, 1},
    };
    
    u32 Tiles11[TILE_MAP_COUNT_Y][TILE_MAP_COUNT_X] =
    {
        {1, 1, 1, 1,  1, 1, 1, 1,  0, 1, 1, 1,  1, 1, 1, 1, 1},
        {1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 1},
        {1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 1},
        {1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 1},
        {0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 1},
        {1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 1},
        {1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 1},
        {1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 1},
        {1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1, 1},
    };

    TileMap tile_map0 = {.tiles = (u32 *)Tiles00};
    TileMap tile_map1 = {.tiles = (u32 *)Tiles10};
    TileMap tile_map2 = {.tiles = (u32 *)Tiles01};
    TileMap tile_map3 = {.tiles = (u32 *)Tiles11};

    TileMap tile_maps[2][2] = {
        {tile_map0, tile_map1},
        {tile_map2, tile_map3}
    };

    World world;
    world.tile_map_count_x = 2;
    world.tile_map_count_y = 2;
    world.count_x = TILE_MAP_COUNT_X;
    world.count_y = TILE_MAP_COUNT_Y;
    world.upper_left_x = -30;
    world.upper_left_y = 0;
    world.tile_width = 60;
    world.tile_height = 60;
    world.tile_maps = (TileMap *)tile_maps;
    f32 player_width = 0.5f*world.tile_width;
    f32 player_height = 0.5f*world.tile_height;

    world.tile_maps = (TileMap *)tile_maps;
    
    GameState *game_state = (GameState *)GameMemory->PermenantStorage;
    if(!GameMemory->IsInitialized) 
    {
        game_state->player_x = 150.f;
        game_state->player_y = 150.f;

        GameMemory->IsInitialized = TRUE;
    }

    TileMap *tile_map = get_tile_map(&world, game_state->player_tile_map_x, game_state->player_tile_map_y);
    Assert(tile_map);

    f32 PlayerR = 1.0f;
    f32 PlayerG = 1.0f;
    f32 PlayerB = 0.0f;

    GameInput *Input = game_input;
    // NOTE(casey): Use digital movement tuning
    f32 d_player_x = 0.0f; // pixels/second
    f32 d_player_y = 0.0f; // pixels/second
                         //
    if(Input->Controller.Up.EndedDown) {
        d_player_y = -1.0f;
    }

    if(Input->Controller.Down.EndedDown) {
        d_player_y = 1.0f;
    }

    if(Input->Controller.Left.EndedDown) {
        d_player_x = -1.0f;
    }

    if(Input->Controller.Right.EndedDown) {
        d_player_x = 1.0f;
    }

    d_player_x *= 256.0f;
    d_player_y *= 256.0f;

    f32 new_player_x = game_state->player_x + Input->dt_for_frame*d_player_x;
    f32 new_player_y = game_state->player_y + Input->dt_for_frame*d_player_y;

    RawPosition player_pos = {
        game_state->player_tile_map_x,
        game_state->player_tile_map_y,
        new_player_x, 
        new_player_y
    };

    RawPosition player_left = player_pos;
    player_left.x -= 0.5f*player_width;

    RawPosition player_right = player_pos;
    player_right.x += 0.5f*player_width;

    u8 has_no_collision = is_world_point_empty(&world, &player_pos) && 
        is_world_point_empty(&world, &player_left) &&
        is_world_point_empty(&world, &player_right);

    if (has_no_collision)
    {
        CanonicalPosition can_pos = get_canonical_world_position(&world, &player_pos);

        game_state->player_tile_map_x = can_pos.tile_map_x;
        game_state->player_tile_map_y = can_pos.tile_map_y;                
        game_state->player_x = world.upper_left_x + world.tile_width*can_pos.tile_x + can_pos.tile_rel_x;
        game_state->player_y = world.upper_left_y + world.tile_height*can_pos.tile_y + can_pos.tile_rel_y;
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
            u32 TileID = get_tile_value_unchecked(&world, tile_map, Column, Row);
            f32 Gray = 0.5f;
            if(TileID == 1)
            {
                Gray = 0.0f;
            }

            f32 MinX = world.upper_left_x + ((f32)Column)*world.tile_width;
            f32 MinY = world.upper_left_y + ((f32)Row)*world.tile_height;
            f32 MaxX = MinX + world.tile_width;
            f32 MaxY = MinY + world.tile_height;
            draw_rectangle(buffer, MinX, MinY, MaxX, MaxY, Gray, Gray, Gray);
        }
    }
    
    // draw the player
    f32 PlayerLeft = game_state->player_x - 0.5f*player_width;
    f32 PlayerTop = game_state->player_y - player_height;
    draw_rectangle(buffer,
                  PlayerLeft, PlayerTop,
                  PlayerLeft + player_width,
                  PlayerTop + player_height,
                  PlayerR, PlayerG, PlayerB);

    /* draw_rectangle(Buffer, 0.0f, 0.0f, (f32)Buffer->Width, (f32)Buffer->Height, 0.4f, .5f, 1.0f); */
    /* draw_rectangle(Buffer, 200.0f, 20.0f, 40.f, 40.f, 1.f, 0.f, 0.f); */
    /* RenderWeirdGradient(*Buffer, GameState->BlueOffset, GameState->GreenOffset); */
}
