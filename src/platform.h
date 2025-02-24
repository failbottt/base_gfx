#ifndef PLATFORM_H
#define PLATFORM_H

#include "base_inc.h"
#include "handmade_math.h"

typedef struct GameOffscreenBuffer
{
    // Pixels are always 32-bits wide, Memory Order BB GG RR XX
    void *Memory;
    int Width;
    int Height;
    int Pitch;
    int BytesPerPixel;
} GameOffscreenBuffer;

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

typedef struct GameInput
{
    game_controller_input Controller;
    f32 dt_for_frame;
} GameInput;

typedef struct GameMemory
{
    u8 IsInitialized;
    u64 PermenantStorageSize;
    void* PermenantStorage; // NOTE(spangler): REQUIRED to be cleared to zero at startup
                            //
    u64 ScratchStorageSize;
    void* ScratchStorage; // NOTE(spangler): REQUIRED to be cleared to zero at startup
} GameMemory;


// NOTE(spangler): services that the game provides to the platform layer
// timing, controller/keyboard input, bitmap buffer to use, sound buffer to use.
void 
GameUpdateAndRender(GameMemory *Memory, GameInput *game_input, GameOffscreenBuffer *buffer);

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
typedef struct GameState {
    /* u64 GreenOffset; */
    /* u64 BlueOffset; */

    u32 player_tile_map_x;
    u32 player_tile_map_y;

    f32 player_x;
    f32 player_y;
} GameState;

#endif // !PLATFORM_H
