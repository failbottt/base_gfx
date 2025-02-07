#include "platform.h"
#include <windows.h>

internal void
RenderWeirdGradient(game_offscreen_buffer Buffer, int BlueOffset, int GreenOffset)
{
    u8 *Row = (u8 *)Buffer.Memory;    
    for(int Y = 0;
        Y < Buffer.Height;
        ++Y)
    {
        u32 *Pixel = (u32 *)Row;
        for(int X = 0;
            X < Buffer.Width;
            ++X)
        {
            u8 Blue = (X + BlueOffset);
            u8 Green = (Y + GreenOffset);

            *Pixel++ = ((Green << 8) | Blue);
        }
        
        Row += Buffer.Pitch;
    }
}

internal u8
DEBUGPlatformWriteEntireFile(char *Filename, u64 MemorySize, void *Memory)
{
    u8 Result = 0;

    HANDLE FileHandle = CreateFile(Filename, GENERIC_WRITE, 0, 0, CREATE_ALWAYS, 0, 0);
    DWORD LastError = GetLastError();

    if(FileHandle != INVALID_HANDLE_VALUE)
    {
        DWORD BytesWritten;
        if(WriteFile(FileHandle, Memory, MemorySize, &BytesWritten, 0) && (MemorySize == BytesWritten))
        {
            // file read success
            Result = (BytesWritten == MemorySize);
        }
        else
        {
            // logging
            DWORD LastError = GetLastError();
        }

        CloseHandle(FileHandle);

    }
    else
    {
        // logging
    }

    return Result;
}

internal File
DEBUGPlatformReadEntireFile(char *Filename)
{
    File Result = {0};

    HANDLE FileHandle = CreateFile(Filename, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);
    DWORD LastError = GetLastError();

    if(FileHandle != INVALID_HANDLE_VALUE)
    {
        LARGE_INTEGER FileSize;
        if(GetFileSizeEx(FileHandle, &FileSize)) {
            Assert(FileSize.QuadPart <= max_u64);
            u64 FileSize64 = FileSize.QuadPart;
            void *Buffer = VirtualAlloc(0, FileSize.QuadPart, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
            if(Buffer)
            {
                DWORD BytesRead;
                // C is required to execute these in order
                // A && B && C
                // will always be A THEN B THEN C
                if(ReadFile(FileHandle, Buffer, FileSize.QuadPart, &BytesRead, 0) &&
                // if we got the file size
                // someone truncated the file
                // we read the file
                // and we didn't read back the expected size we know something went wrong
                    (FileSize64 == BytesRead))
                {
                    // file read success
                    Result.Length = BytesRead;
                    Result.Data = Buffer;
                }
                else
                {
                    // logging
                    DEBUGPlatformFreeFileMemory(Result, FileSize64);
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

internal void
DEBUGPlatformFreeFileMemory(void *Memory, u64 MemorySize)
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

internal void
GameUpdateAndRender(game_memory *Memory, game_input *GameInput, game_offscreen_buffer *Buffer)
{
    game_state *GameState = (game_state *)Memory->PermenantStorage;
    Assert(sizeof(game_state) <= Memory->PermenantStorageSize);
    if(!Memory->IsInitialized) 
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

        char *Filename = "../assets/images/test.png";
        File BitmapMemory = DEBUGPlatformReadEntireFile(Filename);
        if(BitmapMemory.Length > 0)
        {
            DEBUGPlatformWriteEntireFile("../assets/foo.png", BitmapMemory.Length, BitmapMemory.Data);
            /* DEBUGPlatformFreeFileMemory(BitmapMemory); */
        }



        GameState->GreenOffset = 0;
        GameState->BlueOffset = 0;
        
        // maybe this should be done on the os side of things
        Memory->IsInitialized = 1;
    }

    game_controller_input *Input = &GameInput->Controller;
    if(Input->Up.EndedDown) {
        GameState->GreenOffset += 1;
    }

    if(Input->Down.EndedDown) {
        GameState->GreenOffset -= 1;
    }

    if(Input->Right.EndedDown) {
        GameState->BlueOffset -= 1;
    }

    if(Input->Left.EndedDown) {
        GameState->BlueOffset += 1;
    }
    RenderWeirdGradient(*Buffer, GameState->BlueOffset, GameState->GreenOffset);
}
