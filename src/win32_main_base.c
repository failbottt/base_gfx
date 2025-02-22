/*
 * TODO(spangler): Platform layer
 *
 * - Saved game locations
 * - Getting a handle to our own executable file
 * - Asset loading path
 * - Threading (launch a thread)
 * - Raw Input (support for multiple keyboards)
 * - ClipCursor() (for multimonitor support);
 * - Fullscreen support
 * - WM_SETCURSOR (control cursor visibility)
 * - QueryCancelAutoplay
 * - WM_ACTIVATEAPP (for when we are not active application)
 * - Blit speed improvements (BitBlt)
 * - Hardware acceleration (OPENGL or Direct3D or BOTH)
 * - GetKeyboardLayout (for French keyboards, international WASD support)
 */

/* Goal of platform layer
 *
 * Easy to create a "optimally performant" port for other platforms
 *
 */

#include <windows.h>
#include <timeapi.h>
#include <stdio.h>
#include <dsound.h>

#include "platform.h"
#include "win32_platform_base.c"

#define SCREEN_WIDTH 960
#define SCREEN_HEIGHT 540

global_variable u64 GlobalPerfCountFrequency;
global_variable int GlobalRunning;

typedef struct Win32OffscreenBuffer
{
    // Pixels are always 32-bits wide, Memory Order BB GG RR XX
    BITMAPINFO Info;
    void *Memory;
    int Width;
    int Height;
    int Pitch;
    int BytresPerPixel;
} Win32OffscreenBuffer;
global_variable Win32OffscreenBuffer global_back_buffer;

typedef struct win32_window_dimension
{
    int Width;
    int Height;
} win32_window_dimension;


win32_window_dimension
Win32GetWindowDimension(HWND Window)
{
    win32_window_dimension Result;
    
    RECT ClientRect;
    GetClientRect(Window, &ClientRect);
    Result.Width = ClientRect.right - ClientRect.left;
    Result.Height = ClientRect.bottom - ClientRect.top;

    return(Result);
}

internal void
Win32ResizeDIBSection(Win32OffscreenBuffer *Buffer, int Width, int Height)
{
    if(Buffer->Memory)
    {
        VirtualFree(Buffer->Memory, 0, MEM_RELEASE);
    }

    Buffer->Width = Width;
    Buffer->Height = Height;

    int BytesPerPixel = 4;
    Buffer->BytresPerPixel = BytesPerPixel;

    // When the biHeight field is negative, this is the clue to
    // Windows to treat this bitmap as top-down, not bottom-up, meaning that
    // the first three bytes of the image are the color for the top left pixel
    // in the bitmap, not the bottom left!
    Buffer->Info.bmiHeader.biSize = sizeof(Buffer->Info.bmiHeader);
    Buffer->Info.bmiHeader.biWidth = Buffer->Width;
    Buffer->Info.bmiHeader.biHeight = -Buffer->Height;
    Buffer->Info.bmiHeader.biPlanes = 1;
    Buffer->Info.bmiHeader.biBitCount = 32;
    Buffer->Info.bmiHeader.biCompression = BI_RGB;

    int BitmapMemorySize = (Buffer->Width*Buffer->Height)*BytesPerPixel;
    Buffer->Memory = VirtualAlloc(0, BitmapMemorySize, MEM_COMMIT, PAGE_READWRITE);
    Buffer->Pitch = Width*BytesPerPixel;

    // TODO: Probably clear this to black
}

internal void
win32_display_buffer_in_window(HDC DeviceContext,
                           int WindowWidth, int WindowHeight,
                           Win32OffscreenBuffer Buffer)
{
    // TODO: Aspect ratio correction
    
    // TODO: Play with stretch modes
    StretchDIBits(DeviceContext,
                  /*
                  X, Y, Width, Height,
                  X, Y, Width, Height,
                  */
                  0, 0, WindowWidth, WindowHeight,
                  0, 0, Buffer.Width, Buffer.Height,
                  Buffer.Memory,
                  &Buffer.Info,
                  DIB_RGB_COLORS, SRCCOPY);
}

internal void
Win32ProcessKeyboardMessage(game_button_state *NewState, u64 IsDown)
{
    Assert(NewState->EndedDown != IsDown);
    NewState->EndedDown = IsDown;
    ++NewState->HalfTransitionCount;
}

void
Win32ProcessPendingMessages(game_controller_input *Keyboard)
{
    MSG Message;
    while(PeekMessage(&Message, 0, 0, 0, PM_REMOVE))
    {
        switch(Message.message)
        {
            case WM_QUIT:
            {
                // TODO: Handle this with a message to the user?
                GlobalRunning = 0;
            } break;

            case WM_SYSKEYDOWN:
            case WM_SYSKEYUP:
            case WM_KEYDOWN:
            case WM_KEYUP:
            {

                u32 VKCode = (u32) Message.wParam;
                int WasDown = ((Message.lParam & (1<<30)) != FALSE); // the previous key state
                int IsDown = ((Message.lParam & (1<<31)) == FALSE); // the previous key state
                
                if (WasDown != IsDown)
                {
                    if (VKCode == 'W')
                    {
                        Win32ProcessKeyboardMessage(&Keyboard->Up, IsDown);
                    } 
                    else if (VKCode == 'A')
                    {
                        Win32ProcessKeyboardMessage(&Keyboard->Left, IsDown);
                    }
                    else if (VKCode == 'S')
                    {
                        Win32ProcessKeyboardMessage(&Keyboard->Down, IsDown);
                    }
                    else if (VKCode == 'D')
                    {
                        Win32ProcessKeyboardMessage(&Keyboard->Right, IsDown);
                    }
                    else if (VKCode == 'E')
                    {

                    }
                    else if (VKCode == 'Q')
                    {

                    }
                    else if (VKCode == VK_UP)
                    {

                    }
                    else if (VKCode == VK_LEFT)
                    {

                    } 
                    else if (VKCode == VK_DOWN)
                    {

                    } 
                    else if (VKCode == VK_RIGHT)
                    {

                    } 
                    else if (VKCode == VK_ESCAPE)
                    {

                    } 
                    else if (VKCode == VK_SPACE)
                    {

                    } 
                }

                u32 AltKeyWasDown = (Message.lParam & (1 << 29));
                if((VKCode == VK_F4) && AltKeyWasDown)
                {
                    GlobalRunning = FALSE;
                }

            } break;
            
            default:
            {
                TranslateMessage(&Message);
                DispatchMessageA(&Message);
            } break;
        }
    }
}

LRESULT CALLBACK
Win32MainWindowCallback(HWND Window,
                        UINT Message,
                        WPARAM WParam,
                        LPARAM LParam)
{       
    LRESULT Result = 0;

    switch(Message)
    {
        case WM_CLOSE:
        {
            // TODO: Handle this with a message to the user?
            GlobalRunning = 0;
        } break;

        case WM_ACTIVATEAPP:
        {
            OutputDebugStringA("WM_ACTIVATEAPP\n");
        } break;

        case WM_DESTROY:
        {
            // TODO: Handle this as an error - recreate window?
            GlobalRunning = 0;
        } break;

        case WM_SYSKEYDOWN:
        case WM_SYSKEYUP:
        case WM_KEYDOWN:
        case WM_KEYUP:
        {
            // Keyboard events don't appear to go through the 
            // window callback function. They go through the PeekMessage loop.
            Assert(!"Keyboard input came through a non-dispatch message!"); 
        } break;

        case WM_PAINT:
        {
            PAINTSTRUCT Paint;
            HDC DeviceContext = BeginPaint(Window, &Paint);
            win32_window_dimension Dimension = Win32GetWindowDimension(Window);
            win32_display_buffer_in_window(DeviceContext, Dimension.Width, Dimension.Height,
                                       global_back_buffer);
            EndPaint(Window, &Paint);
        } break;

        default:
        {
            Result = DefWindowProc(Window, Message, WParam, LParam);
        } break;
    }
    
    return(Result);
}

inline LARGE_INTEGER
win32_get_wall_clock()
{
    LARGE_INTEGER Result;
    QueryPerformanceCounter(&Result);
    return Result;
}

inline f32
win32_get_seconds_elapsed(LARGE_INTEGER Start, LARGE_INTEGER End)
{
        f32 Result = ((f32)(End.QuadPart - Start.QuadPart)/ 
                        (f32)GlobalPerfCountFrequency);
        return Result;
}

/* #define DIRECT_SOUND_CREATE(name) HRESULT WINAPI name(LPCGUID pcGuidDevice, LPDIRECTSOUND *ppDS, LPUNKNOWN pUnkOuter); */
/* typedef DIRECT_SOUND_CREATE(direct_sound_create); */

/* internal void */
/* Win32InitDirectSound(HWND Window, u64 SamplesPerSecond, u64 BufferSize) */
/* { */
/*     // NOTE(casey): Load the library */
/*     HMODULE DSoundLibrary = LoadLibraryA("dsound.dll"); */
/*     if(DSoundLibrary) */
/*     { */
/*         // NOTE(casey): Get a DirectSound object! - cooperative */
/*         direct_sound_create *DirectSoundCreate = (direct_sound_create *) */
/*             GetProcAddress(DSoundLibrary, "DirectSoundCreate"); */

/*         // TODO(casey): Double-check that this works on XP - DirectSound8 or 7?? */
/*         LPDIRECTSOUND DirectSound; */
/*         if(DirectSoundCreate && SUCCEEDED(DirectSoundCreate(0, &DirectSound, 0))) */
/*         { */
/*             WAVEFORMATEX WaveFormat = {}; */
/*             WaveFormat.wFormatTag = WAVE_FORMAT_PCM; */
/*             WaveFormat.nChannels = 2; */
/*             WaveFormat.nSamplesPerSec = SamplesPerSecond; */
/*             WaveFormat.wBitsPerSample = 16; */
/*             WaveFormat.nBlockAlign = (WaveFormat.nChannels*WaveFormat.wBitsPerSample) / 8; */
/*             WaveFormat.nAvgBytesPerSec = WaveFormat.nSamplesPerSec*WaveFormat.nBlockAlign; */
/*             WaveFormat.cbSize = 0; */

/*             if(SUCCEEDED(DirectSound->SetCooperativeLevel(Window, DSSCL_PRIORITY))) */
/*             { */
/*                 DSBUFFERDESC BufferDescription = {}; */
/*                 BufferDescription.dwSize = sizeof(BufferDescription); */
/*                 BufferDescription.dwFlags = DSBCAPS_PRIMARYBUFFER; */

/*                 // NOTE(casey): "Create" a primary buffer */
/*                 // TODO(casey): DSBCAPS_GLOBALFOCUS? */
/*                 LPDIRECTSOUNDBUFFER PrimaryBuffer; */
/*                 if(SUCCEEDED((DirectSound)->SetCooperativeLevel()) */
/*                 { */
/*                     HRESULT Error = PrimaryBuffer->SetFormat(&WaveFormat); */
/*                     if(SUCCEEDED(Error)) */
/*                     { */
/*                         // NOTE(casey): We have finally set the format! */
/*                         OutputDebugStringA("Primary buffer format was set.\n"); */
/*                     } */
/*                     else */
/*                     { */
/*                         // TODO(casey): Diagnostic */
/*                     } */
/*                 } */
/*                 else */
/*                 { */
/*                     // TODO(casey): Diagnostic */
/*                 } */
/*             } */
/*             else */
/*             { */
/*                 // TODO(casey): Diagnostic */
/*             } */

/*             // TODO(casey): DSBCAPS_GETCURRENTPOSITION2 */
/*             DSBUFFERDESC BufferDescription = {}; */
/*             BufferDescription.dwSize = sizeof(BufferDescription); */
/*             BufferDescription.dwFlags = 0; */
/*             BufferDescription.dwBufferBytes = BufferSize; */
/*             BufferDescription.lpwfxFormat = &WaveFormat; */
/*             LPDIRECTSOUNDBUFFER SecondaryBuffer; */
/*             HRESULT Error = DirectSound->CreateSoundBuffer(&BufferDescription, &SecondaryBuffer, 0); */
/*             if(SUCCEEDED(Error)) */
/*             { */
/*                 OutputDebugStringA("Secondary buffer created successfully.\n"); */
/*             } */
/*         } */
/*         else */
/*         { */
/*             // TODO(casey): Diagnostic */
/*         } */
/*     } */
/*     else */
/*     { */
/*         // TODO(casey): Diagnostic */
/*     } */
/* } */



int CALLBACK
WinMain(HINSTANCE Instance,
        HINSTANCE PrevInstance,
        LPSTR CommandLine,
        int ShowCode)
{
    LARGE_INTEGER PerfCountFrequencyResult;
    QueryPerformanceFrequency(&PerfCountFrequencyResult);
    GlobalPerfCountFrequency = PerfCountFrequencyResult.QuadPart;

    // NOTE sset window scheduler granularity to 1 ms
    // so that our Sleep() can be more granular
    UINT DesiredSchedulerMS = 1;
    u64 SleepIsGranular = (timeBeginPeriod(DesiredSchedulerMS) == TIMERR_NOERROR);


    WNDCLASS WindowClass = {};

    // TODO: How do we reliably get the info about the current monitor from windows?
    int MonitorRefreshHz = 60;
    int GameUpdateHz = MonitorRefreshHz;
    f32 TargetSecondsPerFrame = 1.0f / (f32)GameUpdateHz;

    Win32ResizeDIBSection(&global_back_buffer, SCREEN_WIDTH, SCREEN_HEIGHT);
    
    WindowClass.style = CS_HREDRAW|CS_VREDRAW|CS_OWNDC;
    WindowClass.lpfnWndProc = Win32MainWindowCallback;
    WindowClass.hInstance = Instance;
    //WindowClass.hIcon;
    WindowClass.lpszClassName = "Failbot Base";

    if(RegisterClassA(&WindowClass))
    {
        HWND Window =
            CreateWindowExA(
                0,
                WindowClass.lpszClassName,
                "Handmade Hero",
                WS_OVERLAPPEDWINDOW|WS_VISIBLE,
                CW_USEDEFAULT,
                CW_USEDEFAULT,
                CW_USEDEFAULT,
                CW_USEDEFAULT,
                0,
                0,
                Instance,
                0);
        if(Window)
        {
            // NOTE: Since we specified CS_OWNDC, we can just
            // get one device context and use it forever because we
            // are not sharing it with anyone.
            HDC DeviceContext = GetDC(Window);

            int XOffset = 0;
            int YOffset = 0;

            /* Win32InitDirectSound(Window, 48000, 48000*sizeof(u16)*2); */

            GlobalRunning = TRUE;

#if BUILD_INTERNAL
            LPVOID BaseAddress = (void *)TB((u64)2);
#else
            LPVOID BaseAddress = 0; 
#endif

            GameMemory Memory = {};
            Memory.PermenantStorageSize = MB(64);
            Memory.ScratchStorageSize = GB((u64)4);

            u64 TotalSize = Memory.PermenantStorageSize + Memory.ScratchStorageSize;
            Memory.PermenantStorage = VirtualAlloc(BaseAddress, TotalSize, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
            Memory.ScratchStorage = ((u8 *)Memory.PermenantStorageSize + Memory.PermenantStorageSize);

            if (Memory.PermenantStorage && Memory.ScratchStorage) 
            {
                GameInput input[2] = {};
                GameInput *new_input = &input[0];
                GameInput *old_input = &input[1];

                new_input->dt_for_frame = TargetSecondsPerFrame;

                LARGE_INTEGER LastCounter = win32_get_wall_clock();

                // __rdtsc is only used for profiling, not game time because it's processor specific.
                u64 LastCycleCount = __rdtsc();
                while(GlobalRunning)
                {
                    game_controller_input *OldKeyboardController = &old_input->Controller;
                    game_controller_input *NewKeyboardController = &new_input->Controller;

                    for (int ButtonIndex = 0; ButtonIndex < ArrayCount(NewKeyboardController->Buttons); ButtonIndex++)
                    {
                        NewKeyboardController->Buttons[ButtonIndex].EndedDown =
                            OldKeyboardController->Buttons[ButtonIndex].EndedDown;
                    }

                    Win32ProcessPendingMessages(NewKeyboardController);

                    GameOffscreenBuffer Buffer = {};
                    Buffer.Memory = global_back_buffer.Memory;
                    Buffer.Width = global_back_buffer.Width;
                    Buffer.Height = global_back_buffer.Height;
                    Buffer.Pitch = global_back_buffer.Pitch;
                    Buffer.BytesPerPixel = global_back_buffer.BytresPerPixel;

                    game_update_and_render(&Memory, new_input, &Buffer);

                    LARGE_INTEGER WorkCounter = win32_get_wall_clock();
                    f32 WorkSecondsElapsed = win32_get_seconds_elapsed(LastCounter, WorkCounter);

                    f32 SecondsElapsedForFrame = WorkSecondsElapsed;
                    if (SecondsElapsedForFrame < TargetSecondsPerFrame) 
                    {
                        while(SecondsElapsedForFrame < TargetSecondsPerFrame)
                        {
                            if(SleepIsGranular)
                            {
                                DWORD SleepMS = (DWORD)(1000.0f * (TargetSecondsPerFrame - SecondsElapsedForFrame));
                                Sleep(SleepMS);
                            }
                            SecondsElapsedForFrame = win32_get_seconds_elapsed(LastCounter, win32_get_wall_clock());
                        }
                    }
                    else
                    {
                        //TODO: missed framerate
                        //TODO: logging
                    }

                    win32_window_dimension Dimension = Win32GetWindowDimension(Window);
                    win32_display_buffer_in_window(DeviceContext, Dimension.Width, Dimension.Height,
                            global_back_buffer);

                    GameInput *temp = new_input;
                    new_input = old_input;
                    old_input = temp;

                    LARGE_INTEGER EndCounter = win32_get_wall_clock();
                    f64 MSPerFrame = (f64)(1000.0f*win32_get_seconds_elapsed(LastCounter, EndCounter));

                    LastCounter = EndCounter;

                    u64 EndCycleCount = __rdtsc();
                    u64 CyclesElapsed = EndCycleCount - LastCycleCount;
                    LastCycleCount = EndCycleCount;
#if 1
                    // (how many elapsed / how many per second = how many seconds elapsed)
                    /* u64 FPS = (u64)(GlobalPerfCountFrequency / CounterElapsed); */
                    f64 FPS = 0.0f;
                    f64 MCPF = (f64)(CyclesElapsed / (1000.0f * 1000.0f)) ;
                    // FPS * MCPF = ~processor speed

                    char buffer[256];
                    _snprintf_s(
                            buffer, 
                            sizeof(buffer), 
                            256,
                            "%.02fms/f,  %.02lff/s,  %.02fmc/f\n", 
                            MSPerFrame, 
                            FPS, 
                            MCPF);
                    OutputDebugStringA(buffer);
#endif
                }
            } 
            else 
            {
                // log that the game couldn't start
                OutputDebugStringA("ERROR: unable to init game");
            }
        }
        else
        {
            // TODO: Logging
        }
    }
    else
    {
        // TODO: Logging
    }
    
    return(0);
}

