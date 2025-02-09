#ifndef PLATFORM_H
#define PLATFORM_H

#include "base_inc.h"
#include "handmade_math.h"

typedef struct game_offscreen_buffer
{
    // Pixels are always 32-bits wide, Memory Order BB GG RR XX
    void *Memory;
    int Width;
    int Height;
    int Pitch;
} game_offscreen_buffer;

typedef struct game_button_state
{
    u64 HalfTransitionCount;
    u64 EndedDown;
} game_button_state;

typedef struct game_controller_input {
    union
    {
        game_button_state Buttons[5];
        struct {
            game_button_state Up;
            game_button_state Down;
            game_button_state Left;
            game_button_state Right;
            game_button_state Space;
        };
    };
} game_controller_input;

typedef struct game_input
{
    game_controller_input Controller;
} game_input;

typedef struct game_memory
{
    u8 IsInitialized;
    u64 PermenantStorageSize;
    void* PermenantStorage; // NOTE(spangler): REQUIRED to be cleared to zero at startup
                            //
    u64 ScratchStorageSize;
    void* ScratchStorage; // NOTE(spangler): REQUIRED to be cleared to zero at startup
} game_memory;


// NOTE(spangler): services that the game provides to the platform layer
// timing, controller/keyboard input, bitmap buffer to use, sound buffer to use.
internal void 
GameUpdateAndRender(game_memory *Memory, game_input *GameInput, game_offscreen_buffer *Buffer);

// NOTE(spangler): services that the platform layer provides to the game
// e.g. LoadFile();

// tmp to get things running
// remove this -- it's controlled by the compiler flags
// but it's throwing editor errors, so this is okay for now...
#define BUILD_INTERNAL 1
#if BUILD_INTERNAL
internal File DEBUGPlatformReadEntireFile(char *Filename);
internal u8 DEBUGPlatformWriteEntireFile(char *Filename, u64 MemorySize, void *Memory);
internal void DEBUGPlatformFreeFileMemory(void *Memory, u64 MemorySize);
#endif

// temp definitions
typedef struct game_state {
    u64 GreenOffset;
    u64 BlueOffset;
} game_state;

#endif // !PLATFORM_H
