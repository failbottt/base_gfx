#ifndef PLATFORM_H
#define PLATFORM_H

#include "base_inc.h"
#include "handmade_math.h"

typedef struct game_offscreen_buffer
{
    // NOTE(casey): Pixels are always 32-bits wide, Memory Order BB GG RR XX
    void *Memory;
    int Width;
    int Height;
    int Pitch;
} game_offscreen_buffer;

// NOTE(spangler): services that the game provides to the platform layer
// timing, controller/keyboard input, bitmap buffer to use, sound buffer to use.
internal void GameUpdateAndRender(game_offscreen_buffer *Buffer, int BlueOffset, int GreenOffset);

// NOTE(spangler): services that the platform layer provides to the game
// e.g. LoadFile();

#endif // !PLATFORM_H
