#include <windows.h>
#include <stdio.h>
#include <assert.h>

#include "DarkEngine/DarkEngine_layer.h"
#include "DarkEngine/DarkEngine_memory.h"
#include "DarkEngine/DarkEngine_platform_interface.h"

#include "win32_platform.h"
#include "util.h"

#define BIG_BOI_ALLOC_SIZE Megabytes(20)

global game_state globalGameState;
global win32_back_buffer globalWin32BackBuffer;
global WINDOWPLACEMENT g_wpPrev = { sizeof(g_wpPrev) };

internal void 
ToggleFullscreen(HWND window)
{
    DWORD style = GetWindowLong(window, GWL_STYLE);
    if((style & WS_OVERLAPPEDWINDOW) && !globalGameState.isFullscreen) {
        MONITORINFO mi = { sizeof(mi) };
        if (GetWindowPlacement(window, &g_wpPrev) &&
            GetMonitorInfo(MonitorFromWindow(window,
                                             MONITOR_DEFAULTTOPRIMARY), &mi)) 
        {
            SetWindowLong(window, GWL_STYLE,
                          style & ~WS_OVERLAPPEDWINDOW);
            SetWindowPos(window, HWND_TOP,
                         mi.rcMonitor.left, mi.rcMonitor.top,
                         mi.rcMonitor.right - mi.rcMonitor.left,
                         mi.rcMonitor.bottom - mi.rcMonitor.top,
                         SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
            globalGameState.isFullscreen = 1;
        }
    } 
    else 
    {
        SetWindowLong(window, GWL_STYLE,
                      style | WS_OVERLAPPEDWINDOW);
        SetWindowPlacement(window, &g_wpPrev);
        SetWindowPos(window, NULL, 0, 0, 0, 0,
                     SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER |
                     SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
        globalGameState.isFullscreen = 0;
    }
}

internal FILETIME 
Win32_GetFileLastModifiedTime(char* filename)
{
    HANDLE fileHandle = CreateFileA(filename,
                                    GENERIC_READ,
                                    0,
                                    0,
                                    OPEN_ALWAYS,
                                    FILE_ATTRIBUTE_NORMAL,
                                    0);
    FILETIME result = {0};
    GetFileTime(fileHandle,
                0,
                0,
                &result);
    
    CloseHandle(fileHandle);
    
    return result;
}

internal game_code
Win32_LoadGameCode(char* dllName)
{
    game_code gameCodeLoad;
    
    char tempDLL[] = "ADarkAdventure_Temp.dll";
    
    CopyFile(dllName,
             tempDLL,
             0);
    
    gameCodeLoad.gameCode = LoadLibraryA(tempDLL);
    gameCodeLoad.Game_UpdateAndRender = 
        (game_update_and_render*)GetProcAddress(gameCodeLoad.gameCode,
                                                "Game_UpdateAndRender");
    if(!gameCodeLoad.Game_UpdateAndRender)
    {
        gameCodeLoad.Game_UpdateAndRender = Game_UpdateAndRenderStub;
    }
    
    return gameCodeLoad;
}

internal void
Win32_UnloadGameCode(game_code* gameCode)
{
    FreeLibrary(gameCode->gameCode);
    gameCode->gameCode = 0;
    gameCode->Game_UpdateAndRender = Game_UpdateAndRenderStub;
}

internal window_dimensions
Win32_GetWindowDimensions(HWND window)
{
    RECT dataRect = {0};
    GetClientRect(window,
                  &dataRect);
    
    window_dimensions dimension = {0};
    dimension.width = (u16)(dataRect.right - dataRect.left);
    dimension.height = (u16)(dataRect.bottom - dataRect.top);
    
    return dimension;
}

internal win32_back_buffer
new_back_buffer(MemoryArena* arena,
                u16 width, 
                u16 height)
{
    win32_back_buffer backBuffer = {0};
    
    backBuffer.width = width;
    backBuffer.height = height;
    
    backBuffer.bitmapInfo.bmiHeader.biSize = sizeof(backBuffer.bitmapInfo.bmiHeader);
    backBuffer.bitmapInfo.bmiHeader.biWidth = backBuffer.width;
    backBuffer.bitmapInfo.bmiHeader.biHeight = -backBuffer.height;
    backBuffer.bitmapInfo.bmiHeader.biPlanes = 1;
    backBuffer.bitmapInfo.bmiHeader.biBitCount = 32;
    backBuffer.bitmapInfo.bmiHeader.biCompression = BI_RGB;
    
    u32 sizeToAlloc = width * height * BYTES_PER_PIXEL;
    backBuffer.memory = ArenaAlloc(arena, sizeToAlloc);
    backBuffer.pitch = width * BYTES_PER_PIXEL;
    
    return backBuffer;
}

internal void
Win32_UpdateWindow(HDC hdc,
                   win32_back_buffer* backBuffer,
                   u16 windowWidth,
                   u16 windowHeight)
{
    StretchDIBits(hdc,
                  0, 0,
                  windowWidth,
                  windowHeight,
                  0, 0,
                  backBuffer->width,
                  backBuffer->height,
                  backBuffer->memory,
                  &backBuffer->bitmapInfo,
                  DIB_RGB_COLORS,
                  SRCCOPY);
}

internal LRESULT 
Win32_DefaultWindowCallback(HWND window,
                            UINT message,
                            WPARAM wParam,
                            LPARAM lParam)
{
    LRESULT result = {0};
    
    switch(message)
    {
        case WM_QUIT:
        {
            PushOSEvent(&globalGameState.eventList,
                        WINDOW_CLOSE);
        } break;
        case WM_CLOSE:
        {
            PushOSEvent(&globalGameState.eventList,
                        WINDOW_CLOSE);
        } break;
        case WM_DESTROY:
        {
            PushOSEvent(&globalGameState.eventList,
                        WINDOW_CLOSE);
        } break;
        case WM_PAINT:
        {
            window_dimensions dimension = Win32_GetWindowDimensions(window);
            
            PAINTSTRUCT paintStruct = {0};
            HDC paintDC = BeginPaint(window,
                                     &paintStruct);
            Win32_UpdateWindow(paintDC,
                               &globalWin32BackBuffer,
                               dimension.width,
                               dimension.height);
            
            EndPaint(window,
                     &paintStruct);
        } break;
        case WM_KEYUP:
        case WM_KEYDOWN:
        {
            u16 VKcode = (u16)wParam;
            b32 isDown = ((lParam) & (1 << 31)) == 0;
            b32 wasDown = ((lParam) & (1 << 30)) != 0;
            if(isDown != wasDown)
            {
                key_code keyCode = KEY_NULL;
                
                switch(VKcode)
                {
                    case 'Q':
                    {
                        keyCode = KEY_Q;
                    } break;
                    case 'W':
                    {
                        keyCode = KEY_W;
                    } break;
                    case 'E':
                    {
                        keyCode = KEY_E;
                    } break;
                    case 'R':
                    {
                        keyCode = KEY_R;
                    } break;
                    case 'T':
                    {
                        keyCode = KEY_T;
                    } break;
                    case 'Y':
                    {
                        keyCode = KEY_Y;
                    } break;
                    case 'U':
                    {
                        keyCode = KEY_U;
                    } break;
                    case 'I':
                    {
                        keyCode = KEY_I;
                    } break;
                    case 'O':
                    {
                        keyCode = KEY_O;
                    } break;
                    case 'P':
                    {
                        keyCode = KEY_P;
                    } break;
                    case 'A':
                    {
                        keyCode = KEY_A;
                    } break;
                    case 'S':
                    {
                        keyCode = KEY_S;
                    } break;
                    case 'D':
                    {
                        keyCode = KEY_D;
                    } break;
                    case 'F':
                    {
                        keyCode = KEY_F;
                    } break;
                    case 'G':
                    {
                        keyCode = KEY_G;
                    } break;
                    case 'H':
                    {
                        keyCode = KEY_H;
                    } break;
                    case 'J':
                    {
                        keyCode = KEY_J;
                    } break;
                    case 'K':
                    {
                        keyCode = KEY_K;
                    } break;
                    case 'L':
                    {
                        keyCode = KEY_L;
                    } break;
                    case 'Z':
                    {
                        keyCode = KEY_Z;
                    } break;
                    case 'X':
                    {
                        keyCode = KEY_X;
                    } break;
                    case 'C':
                    {
                        keyCode = KEY_C;
                    } break;
                    case 'V':
                    {
                        keyCode = KEY_V;
                    } break;
                    case 'B':
                    {
                        keyCode = KEY_B;
                    } break;
                    case 'N':
                    {
                        keyCode = KEY_N;
                    } break;
                    case 'M':
                    {
                        keyCode = KEY_M;
                    } break;
                    case VK_UP:
                    {
                        keyCode = KEY_UP;
                    } break;
                    case VK_LEFT:
                    {
                        keyCode = KEY_LEFT;
                    } break;
                    case VK_RIGHT:
                    {
                        keyCode = KEY_RIGHT;
                    } break;
                    case VK_DOWN:
                    {
                        keyCode = KEY_DOWN;
                    } break;
                    case VK_ESCAPE:
                    {
                        keyCode = KEY_ESC;
                    } break;
                    case VK_SPACE:
                    {
                        keyCode = KEY_SPACE;
                    } break;
                    case VK_F11:
                    {
                        if(isDown)
                            ToggleFullscreen(window);
                    } break;
                }
                
                if(isDown)
                {
                    PushOSKeyEvent(&globalGameState.eventList,
                                   KEY_PRESS,
                                   keyCode);
                }
                else
                {
                    PushOSKeyEvent(&globalGameState.eventList,
                                   KEY_RELEASE,
                                   keyCode);
                }
            }
        } break;
        default:
        {
            result = DefWindowProc(window, message, wParam, lParam);
        } break;
    }
    
    return result;
}

internal void
Win32_ProcessMessageQueue(HWND window)
{
    MSG message;
    while(PeekMessageA(&message, window, 0, 0, PM_REMOVE))
    {
        TranslateMessage(&message);
        DispatchMessage(&message);
    }
}

inline f32 
GetTime_MS(LARGE_INTEGER performanceFrequency)
{
    LARGE_INTEGER time;
    QueryPerformanceCounter(&time);
    
    f64 MS_elapsed = 
        ((f64)time.QuadPart / (f64)performanceFrequency.QuadPart) * 1000;
    
    return (f32)MS_elapsed;
}

int
WinMain(HINSTANCE hInstance, 
        HINSTANCE prevInstance,
        LPSTR CMDLine, 
        INT cmdShow)
{
    setvbuf(stdout, 0, _IONBF, 0);
    setvbuf(stderr, 0, _IONBF, 0);
    
    
    // NOTE(winston): Big boi memory alloc
    void* memory = VirtualAlloc(0,
                                BIG_BOI_ALLOC_SIZE,
                                MEM_COMMIT,
                                PAGE_READWRITE);
    
    MemoryArena arena = new_arena(memory, BIG_BOI_ALLOC_SIZE);
    
    globalWin32BackBuffer = new_back_buffer(&arena, 1280, 720);
    
    WNDCLASSA windowClass = {0};
    windowClass.style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
    windowClass.lpfnWndProc = Win32_DefaultWindowCallback;
    windowClass.hInstance = hInstance;
    windowClass.hCursor = LoadCursor(0, IDC_ARROW);
    windowClass.lpszClassName = "Window Class";
    
    
    LARGE_INTEGER performanceFrequency;
    QueryPerformanceFrequency(&performanceFrequency);
    
    if(RegisterClass(&windowClass))
    {
        HWND window = 
            CreateWindowExA(0,
                            "Window Class",
                            "A Dark Adventure",
                            WS_OVERLAPPEDWINDOW | WS_VISIBLE,
                            CW_USEDEFAULT,
                            CW_USEDEFAULT,
                            1280,
                            720,
                            0,
                            0,
                            0,
                            0);
        if(window)
        {
            globalGameState.isRunning = 1;
            globalGameState.eventList = GenerateEventList(&arena, 64);
            
            f32 lastTime = GetTime_MS(performanceFrequency);
            
            back_buffer backBuffer = {0};
            backBuffer.memory = globalWin32BackBuffer.memory;
            backBuffer.height = globalWin32BackBuffer.height;
            backBuffer.width = globalWin32BackBuffer.width;
            backBuffer.pitch = globalWin32BackBuffer.pitch;
            
            HDC hdc = GetDC(window);
            
            game_code gameCode = Win32_LoadGameCode("ADarkAdventure.dll");
            
            FILETIME lastWriteTime = Win32_GetFileLastModifiedTime("ADarkAdventure.dll");
            
            while(globalGameState.isRunning)
            {
                FILETIME currentWriteTime = Win32_GetFileLastModifiedTime("ADarkAdventure.dll");
                
                if(CompareFileTime(&lastWriteTime,
                                   &currentWriteTime))
                {
                    Win32_UnloadGameCode(&gameCode);
                    gameCode = Win32_LoadGameCode("ADarkAdventure.dll");
                    lastWriteTime = currentWriteTime;
                }
                
                Win32_ProcessMessageQueue(window);
                
                gameCode.Game_UpdateAndRender(&globalGameState,
                                              &backBuffer,
                                              &arena);
                
                window_dimensions dimension = Win32_GetWindowDimensions(window);
                
#if 1
                Win32_UpdateWindow(hdc,
                                   &globalWin32BackBuffer,
                                   dimension.width,
                                   dimension.height);
#endif 
                //UpdateWindow(window);
                
                f32 currentTime = GetTime_MS(performanceFrequency);
                f32 timeElapsed = currentTime - lastTime;
                
                //printf("%f\n", timeElapsed);
                if(timeElapsed < 16.667f)
                {
                    Sleep((DWORD)(16.667f - timeElapsed));
                }
                
                lastTime = currentTime;
            }
            
            DestroyWindow(window);
        }
        else
        {
            // TODO(winston): logging
            fprintf(stderr, "Failed to create window.\n");
        }
    }
    else
    {
        // TODO(winston): logging
        fprintf(stderr, "Failed to register window class.\n");
    }
    
    VirtualFree(arena.memory, 0, MEM_RELEASE);
    
    return 0;
}