
#include <stdint.h>
#include <stdio.h>

#define GLOBAL        static // Explicit for global variables
#define LOCAL_PERSIST static // Explicit for locally persisting variables
#define INTERNAL      static // Explicit for functions internal to the translation unit

#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))

typedef uint8_t  U8;
typedef uint16_t U16;
typedef uint32_t U32;
typedef uint64_t U64;

typedef int8_t   S8;
typedef int16_t  S16;
typedef int32_t  S32;
typedef int64_t  S64;

typedef int32_t  B32;
typedef float    R32;
typedef double   R64;

#include <windows.h>
#include "mymath.h"

struct Win32_Bitmap {
BITMAPINFO  info;
void       *memory;
int         width;
int         height;
int         pitch;
int         bytes_per_pixel;
};


GLOBAL bool         g_running;
GLOBAL Win32_Bitmap g_bitmap;
GLOBAL S64          g_tick_frequency;

struct Win32_Window_Dimension {
    int width;
    int height;
};

Win32_Window_Dimension Win32GetWindowDimension(HWND window) {
    Win32_Window_Dimension result;
    RECT client_rect;
    GetClientRect(window, &client_rect);
    result.width = client_rect.right - client_rect.left;
    result.height = client_rect.bottom - client_rect.top;
    return result;
}

INTERNAL void RenderGradient(Win32_Bitmap bitmap, int x_offset, int y_offset) {
    U8 *row = (U8 *)bitmap.memory;
    for (int y = 0; y < bitmap.height; y++) { 
        U32 *pixel = (U32 *)row;
        for (int x = 0; x < bitmap.width; x++) { 

            U8 blue = x + x_offset;
            U8 green = y + y_offset;

            *pixel++ = ((green << 8) | blue);
        }

        row += bitmap.pitch;
    }
}

INTERNAL void Win32ResizeDIBSection(Win32_Bitmap *bitmap, int width, int height) {
    if (bitmap->memory) {
        VirtualFree(bitmap->memory, 0, MEM_RELEASE);
    }

    bitmap->width  = width;
    bitmap->height = height;
    bitmap->bytes_per_pixel = 4;

    bitmap->info.bmiHeader.biSize        = sizeof(bitmap->info.bmiHeader);
    bitmap->info.bmiHeader.biWidth       = bitmap->width;
    bitmap->info.bmiHeader.biHeight      = -bitmap->height;
    bitmap->info.bmiHeader.biPlanes      = 1;
    bitmap->info.bmiHeader.biBitCount    = 32;
    bitmap->info.bmiHeader.biCompression = BI_RGB;

    int bitmap_memory_size = bitmap->width * bitmap->height * bitmap->bytes_per_pixel;
    bitmap->memory = VirtualAlloc(0, bitmap_memory_size, MEM_COMMIT, PAGE_READWRITE);
    bitmap->pitch  = bitmap->width * bitmap->bytes_per_pixel;
}

INTERNAL void Win32CopyBitmapToWindow(HDC device_context, Win32_Bitmap bitmap, 
                                      int window_width, int window_height) { 
    StretchDIBits(device_context, 
                  0, 0, window_width, window_height, 0, 0, bitmap.width, bitmap.height, bitmap.memory, 
                  &bitmap.info, DIB_RGB_COLORS, SRCCOPY);
}

LRESULT CALLBACK Win32MainWindowCallback(HWND window, UINT message, WPARAM w_param, LPARAM l_param) {
    LRESULT result = 0;
    switch (message) {
        case WM_SIZE: {
        } break;
        case WM_DESTROY:     g_running = false;                      break; // TODO: Handle this as an error and maybe recreate the window.
        case WM_CLOSE:       g_running = false;                      break; // TODO: Handle this with a message to the user.
        case WM_ACTIVATEAPP: OutputDebugStringA("WM_ACTIVATEAPP\n"); break;
        case WM_PAINT: {  
            PAINTSTRUCT paint;
            HDC device_context = BeginPaint(window, &paint);
            Win32_Window_Dimension dimension = Win32GetWindowDimension(window);
            Win32CopyBitmapToWindow(device_context, g_bitmap, dimension.width, dimension.height);
            EndPaint(window, &paint);
        } break;
        default: result = DefWindowProc(window, message, w_param, l_param); break;
    }
    return result;
}

INTERNAL LARGE_INTEGER Win32GetTickCount() {
    LARGE_INTEGER result; 
    QueryPerformanceCounter(&result);
    return result;
}

INTERNAL R32 Win32GetSecondsElapsed(LARGE_INTEGER start, LARGE_INTEGER end) {
    R32 result = (R32)(end.QuadPart - start.QuadPart) / (R32)g_tick_frequency;
    return result;
}

INTERNAL S32 RoundR32ToS32(R32 a) {
    S32 result = (S32)(a + 0.5f);
    return result;
}

INTERNAL void DrawRectangle(Win32_Bitmap *bitmap, V2 min, V2 max, U32 color) {
    S32 min_x = RoundR32ToS32(min.x);
    S32 min_y = RoundR32ToS32(min.y);
    S32 max_x = RoundR32ToS32(max.x);
    S32 max_y = RoundR32ToS32(max.y);

    if (min_x < 0)             min_x = 0;
    if (min_y < 0)             min_y = 0;
    if (max_x > bitmap->width)  max_x = bitmap->width;
    if (max_y > bitmap->height) max_y = bitmap->height;

    U8 *row = (U8 *)bitmap->memory + min_x*bitmap->bytes_per_pixel + min_y*bitmap->pitch;
    for (int y = min_y; y < max_y; y++) {
        U32 *pixel = (U32 *)row;
        for (int x = min_x; x < max_x; x++) { 
            *pixel++ = color;
        }
        row += bitmap->pitch;
    }
}

INTERNAL S32 SignedArea(V2 origin, V2 point1, V2 point2) {
    S32 result;
    V2 vector1 = point1 - origin;
    V2 vector2 = point2 - origin;
    result = (vector1.x*vector2.y) - (vector1.y*vector2.x);
    return result;
}

INTERNAL B32 IsLeftOrTopEdge(V2 origin_vertex, V2 next_vertex) {
    B32 result;
    V2 edge = next_vertex - origin_vertex;
    B32 is_left_edge = edge.y > 0;
    B32 is_top_edge  = (edge.y == 0) && (edge.x < 0);
    result = is_left_edge || is_top_edge;
    return result;
}

INTERNAL void DrawTriangle(Win32_Bitmap *bitmap, V2 vert0, V2 vert1, V2 vert2, V4 col0, V4 col1, V4 col2) {
    S32 min_x = RoundR32ToS32(MIN(MIN(vert0.x, vert1.x), vert2.x));
    S32 min_y = RoundR32ToS32(MIN(MIN(vert0.y, vert1.y), vert2.y));
    S32 max_x = RoundR32ToS32(MAX(MAX(vert0.x, vert1.x), vert2.x));
    S32 max_y = RoundR32ToS32(MAX(MAX(vert0.y, vert1.y), vert2.y));

    if (min_x < 0)              min_x = 0;
    if (min_y < 0)              min_y = 0;
    if (max_x > bitmap->width)  max_x = bitmap->width;
    if (max_y > bitmap->height) max_y = bitmap->height;

    S32 bias0 = IsLeftOrTopEdge(vert1, vert2) ? 0 : 1;
    S32 bias1 = IsLeftOrTopEdge(vert2, vert0) ? 0 : 1;
    S32 bias2 = IsLeftOrTopEdge(vert0, vert1) ? 0 : 1;

    R32 triangle_area = (R32)SignedArea(vert0, vert1, vert2);

    U8 *row = (U8 *)bitmap->memory + min_x*bitmap->bytes_per_pixel + min_y*bitmap->pitch;
    for (int y = min_y; y < max_y; y++) {
        U32 *pixel = (U32 *)row;
        for (int x = min_x; x < max_x; x++) {
            V2 pixel_coord = {(R32)x, (R32)y};
            S32 weight0 = SignedArea(vert1, vert2, pixel_coord) + bias0;
            S32 weight1 = SignedArea(vert2, vert0, pixel_coord) + bias1;
            S32 weight2 = SignedArea(vert0, vert1, pixel_coord) + bias2;

            if (weight0 <= 0 && weight1 <= 0 && weight2 <= 0) {
                R32 ratio0 = weight0 / triangle_area;
                R32 ratio1 = weight1 / triangle_area;
                R32 ratio2 = weight2 / triangle_area;

                R32 r = (ratio0*col0.r) + (ratio1*col1.r) + (ratio2*col2.r);
                R32 g = (ratio0*col0.g) + (ratio1*col1.g) + (ratio2*col2.g);
                R32 b = (ratio0*col0.b) + (ratio1*col1.b) + (ratio2*col2.b);


                U8 red   = r * 255.0f;
                U8 green = g * 255.0f;
                U8 blue  = b * 255.0f;

                U32 col = ((red << 16) | (green << 8) | blue);

                *pixel = col;
            }
            pixel++;
        }
        row += bitmap->pitch;
    }
}

int CALLBACK WinMain(HINSTANCE instance, HINSTANCE prev_instance, LPSTR command_line, int show_command_line) {
    LARGE_INTEGER tick_frequency_result;
    QueryPerformanceFrequency(&tick_frequency_result);
    g_tick_frequency = tick_frequency_result.QuadPart;

    WNDCLASS window_class    = {};
    Win32ResizeDIBSection(&g_bitmap, 1280, 720);
    window_class.style       = CS_HREDRAW|CS_VREDRAW|CS_OWNDC;
    window_class.lpfnWndProc = Win32MainWindowCallback;
    window_class.hInstance   = instance;
    // window_class.hIcon;
    window_class.lpszClassName = "RasterWindowClass";

    R32 target_seconds_per_frame = 1.0f / 60.0f;
    // Have to set the millisecond cound of the schedular granularity.
    int desired_schedular_ms = 1;
    timeBeginPeriod(desired_schedular_ms);


    if (RegisterClass(&window_class)) { 
        HWND window = CreateWindowEx(0, window_class.lpszClassName, "Rasteriser", 
                                     WS_OVERLAPPEDWINDOW|WS_VISIBLE, CW_USEDEFAULT,
                                     CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, 
                                     0, 0, instance, 0);
        if (window) {
            HDC device_context = GetDC(window);
            int x_offset = 0;
            int y_offset = 0;
            g_running = true;
            LARGE_INTEGER begin_frame_count = Win32GetTickCount();
            U64 begin_cycle_count = __rdtsc();
            while(g_running) {
                MSG message; 
                while (PeekMessage(&message, 0, 0, 0, PM_REMOVE)) {
                    if (message.message == WM_QUIT) { 
                        g_running = false;
                    } 

                    TranslateMessage(&message);
                    DispatchMessageA(&message);
                }
                //RenderGradient(g_bitmap, x_offset, y_offset);
                U32 purple = 0x00FF00FF;
                U32 white  = 0xFFFFFFFF;
                V4  col0   = {0.03f, 0.52f, 0.63f, 1.0f};
                V4  col1   = {0.46f, 0.12f, 0.62f, 1.0f};
                V4  col2   = {0.63f, 0.82f, 0.05f, 1.0f};
                V4  red    = {0.83f, 0.93f, 0.49f, 1.0f};
                V4  green  = {0.85f, 0.7f,  0.05f, 1.0f};
                V4  blue   = {0.06f, 0.50f, 0.80f, 1.0f};

                DrawRectangle(&g_bitmap, V2{0.0f, 0.0f}, V2{(R32)g_bitmap.width, (R32)g_bitmap.height}, purple);
                //DrawRectangle(&g_bitmap, V2{20.0f, 20.0f}, V2{21.0f, 21.0f}, 0x0000FF00);
                DrawTriangle(&g_bitmap, V2{100, 600}, V2{300, 600}, V2{200, 200}, col0, col1, col2);
                DrawTriangle(&g_bitmap, V2{500, 450}, V2{200, 200}, V2{300, 600}, red, green, blue);
                Win32_Window_Dimension dimension = Win32GetWindowDimension(window);
                Win32CopyBitmapToWindow(device_context, g_bitmap, dimension.width, dimension.height);

                x_offset++;
                y_offset++;

                LARGE_INTEGER work_count = Win32GetTickCount();
                R32 work_seconds = Win32GetSecondsElapsed(begin_frame_count, work_count);

                R32 frame_seconds_elapsed = work_seconds;
                if (frame_seconds_elapsed < target_seconds_per_frame) {
                    S32 ms_to_sleep = (S32)(1000.0f * (target_seconds_per_frame - frame_seconds_elapsed) - 1.0f);
                    if (ms_to_sleep > 0) {
                        Sleep((DWORD)ms_to_sleep);
                    }
                    while (frame_seconds_elapsed < target_seconds_per_frame) {
                        frame_seconds_elapsed = Win32GetSecondsElapsed(begin_frame_count, Win32GetTickCount());
                    }
                } else {
                    // Missed the frame and need to log or sumfin
                }

                U64 end_cycle_count = __rdtsc();
                LARGE_INTEGER current_ticks = Win32GetTickCount();
                R32 fps = (R32)g_tick_frequency / (current_ticks.QuadPart - begin_frame_count.QuadPart); 
                R64 cycles_this_frame = end_cycle_count - begin_cycle_count;
                R32 mega_cycles_per_frame = cycles_this_frame / (1000.0f*1000.0f);
                R32 ms_per_frame = 1000.0f*frame_seconds_elapsed;

                char buffer[256];
                sprintf(buffer, "%.02fms,    %.02ffps,    %.02fcycles\n", ms_per_frame, fps, mega_cycles_per_frame);
                OutputDebugStringA(buffer);

                begin_frame_count = current_ticks;
                begin_cycle_count = end_cycle_count;
            }
        } else {
            // TODO: Do some logging here for failed window handle.
        }
    } else {
            // TODO: Do some logging here for failed window class.
    }
}

