/*
 * TODO(spangler): Platform layer
 *
 * - Saved game locations
 * - Getting a handle to our own executable file
 * - Asset loading path
 * - Threading (launch a thread)
 * - Raw Input (support for multiple keyboards)
 * - Sleep/timeBeginPeriod (don't melt battery)
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

#include "win32_platform.c"

typedef struct win32_offscreen_buffer
{
    // NOTE(casey): Pixels are always 32-bits wide, Memory Order BB GG RR XX
    BITMAPINFO Info;
    void *Memory;
    int Width;
    int Height;
    int Pitch;
} win32_offscreen_buffer;

typedef struct win32_window_dimension
{
    int Width;
    int Height;
} win32_window_dimension;

global_variable int GlobalRunning;
global_variable win32_offscreen_buffer GlobalBackbuffer;

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

/* internal void */
/* RenderWeirdGradient(win32_offscreen_buffer Buffer, int BlueOffset, int GreenOffset) */
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

internal void
Win32ResizeDIBSection(win32_offscreen_buffer *Buffer, int Width, int Height)
{
    if(Buffer->Memory)
    {
        VirtualFree(Buffer->Memory, 0, MEM_RELEASE);
    }

    Buffer->Width = Width;
    Buffer->Height = Height;

    int BytesPerPixel = 4;

    // NOTE(casey): When the biHeight field is negative, this is the clue to
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

    // TODO(casey): Probably clear this to black
}

internal void
Win32DisplayBufferInWindow(HDC DeviceContext,
                           int WindowWidth, int WindowHeight,
                           win32_offscreen_buffer Buffer)
{
    // TODO(casey): Aspect ratio correction
    
    // TODO(casey): Play with stretch modes
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
            // TODO(casey): Handle this with a message to the user?
            GlobalRunning = 0;
        } break;

        case WM_ACTIVATEAPP:
        {
            OutputDebugStringA("WM_ACTIVATEAPP\n");
        } break;

        case WM_DESTROY:
        {
            // TODO(casey): Handle this as an error - recreate window?
            GlobalRunning = 0;
        } break;

        case WM_SYSKEYDOWN:
        case WM_SYSKEYUP:
        case WM_KEYDOWN:
        case WM_KEYUP:
        {
            u32 VKCode = WParam;
            int WasDown = ((LParam & (1<<30)) != FALSE); // the previous key state
            int IsDown = ((LParam & (1<<31)) == FALSE); // the previous key state

            if (WasDown != IsDown)
            {
                if (VKCode == 'W')
                {
                    OutputDebugStringA("W Pressed");
                } 
                else if (VKCode == 'A')
                {
                    OutputDebugStringA("A Pressed");
                }
                else if (VKCode == 'S')
                {
                    OutputDebugStringA("S Pressed");
                }
                else if (VKCode == 'D')
                {
                    OutputDebugStringA("D Pressed");
                }
                else if (VKCode == 'E')
                {
                    OutputDebugStringA("E Pressed");
                }
                else if (VKCode == 'Q')
                {
                    OutputDebugStringA("Q Pressed");
                }
                else if (VKCode == VK_UP)
                {
                    OutputDebugStringA("Up Pressed");
                }
                else if (VKCode == VK_LEFT)
                {
                    OutputDebugStringA("Left Pressed");
                } 
                else if (VKCode == VK_DOWN)
                {
                    OutputDebugStringA("Down Pressed");
                } 
                else if (VKCode == VK_RIGHT)
                {
                    OutputDebugStringA("Right Pressed");
                } 
                else if (VKCode == VK_ESCAPE)
                {
                    OutputDebugStringA("Escape: ");
                    if (IsDown)
                    {
                        OutputDebugStringA("IsDown");
                    } 
                    else if (WasDown)
                    {
                        OutputDebugStringA("WasDown");
                    }
                    OutputDebugStringA("\n");
                } 
                else if (VKCode == VK_SPACE)
                {
                    OutputDebugStringA("Escape Pressed");
                } 
            }

        } break;
        
        case WM_PAINT:
        {
            PAINTSTRUCT Paint;
            HDC DeviceContext = BeginPaint(Window, &Paint);
            win32_window_dimension Dimension = Win32GetWindowDimension(Window);
            Win32DisplayBufferInWindow(DeviceContext, Dimension.Width, Dimension.Height,
                                       GlobalBackbuffer);
            EndPaint(Window, &Paint);
        } break;

        default:
        {
            Result = DefWindowProc(Window, Message, WParam, LParam);
        } break;
    }
    
    return(Result);
}

int CALLBACK
WinMain(HINSTANCE Instance,
        HINSTANCE PrevInstance,
        LPSTR CommandLine,
        int ShowCode)
{

    WNDCLASS WindowClass = {};

    Win32ResizeDIBSection(&GlobalBackbuffer, 1280, 720);
    
    WindowClass.style = CS_HREDRAW|CS_VREDRAW|CS_OWNDC;
    WindowClass.lpfnWndProc = Win32MainWindowCallback;
    WindowClass.hInstance = Instance;
//    WindowClass.hIcon;
    WindowClass.lpszClassName = "Failbot Base";

    LARGE_INTEGER PerfCountFrequencyResult;
    QueryPerformanceFrequency(&PerfCountFrequencyResult);
    u64 PerfCountFrequency = PerfCountFrequencyResult.QuadPart;
    u64 LastCycleCount = __rdtsc();

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
            // NOTE(casey): Since we specified CS_OWNDC, we can just
            // get one device context and use it forever because we
            // are not sharing it with anyone.
            HDC DeviceContext = GetDC(Window);

            int XOffset = 0;
            int YOffset = 0;

            GlobalRunning = TRUE;

            LARGE_INTEGER LastCounter;
            QueryPerformanceCounter(&LastCounter);
            
            while(GlobalRunning)
            {
                LARGE_INTEGER BeginCounter;
                QueryPerformanceCounter(&BeginCounter);

                MSG Message;

                while(PeekMessage(&Message, 0, 0, 0, PM_REMOVE))
                {
                    if(Message.message == WM_QUIT)
                    {
                        GlobalRunning = 0;
                    }
                    
                    TranslateMessage(&Message);
                    DispatchMessageA(&Message);
                }

                game_offscreen_buffer Buffer = {};
                Buffer.Memory = GlobalBackbuffer.Memory;
                Buffer.Width = GlobalBackbuffer.Width;
                Buffer.Height = GlobalBackbuffer.Height;
                Buffer.Pitch = GlobalBackbuffer.Pitch;
                GameUpdateAndRender(&Buffer, XOffset, YOffset);

                win32_window_dimension Dimension = Win32GetWindowDimension(Window);
                Win32DisplayBufferInWindow(DeviceContext, Dimension.Width, Dimension.Height,
                                           GlobalBackbuffer);

                // how many
                u64 EndCycleCount = __rdtsc();
                u64 CyclesElapsed = EndCycleCount - LastCycleCount;

                // MS per frame
                LARGE_INTEGER EndCounter;
                QueryPerformanceCounter(&EndCounter);
                u64 CounterElasped = EndCounter.QuadPart - LastCounter.QuadPart;
                 // (how many elapsed / how many per second = how many seconds elapsed)
                u32 MSPerFrame = (u32)((1000*CounterElasped) / PerfCountFrequency);
                u32 FPS = (PerfCountFrequency / CounterElasped);
                u32 MCPF = (u32)(CyclesElapsed / (1000 * 1000)) ;
                // FPS * MCPF = ~processor speed

                char buffer[256];
                wsprintf(buffer, "%dms/f, %df/s, %dmc/f\n", MSPerFrame, FPS, MCPF);
                OutputDebugStringA(buffer);

                LastCounter = EndCounter;
                LastCycleCount = EndCycleCount;
            }
        }
        else
        {
            // TODO(casey): Logging
        }
    }
    else
    {
        // TODO(casey): Logging
    }
    
    return(0);
}

