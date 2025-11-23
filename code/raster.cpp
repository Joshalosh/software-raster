
#include <windows.h>
#include <stdint.h>

#define GLOBAL        static // Explicit for global variables
#define LOCAL_PERSIST static // Explicit for locally persisting variables
#define INTERNAL      static // Explicit for functions internal to the translation unit

typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int8_t  s8;
typedef int16_t s16;
typedef int32_t s32;
typedef int64_t s64;

GLOBAL bool        g_running;
GLOBAL BITMAPINFO  g_bitmap_info;
GLOBAL void       *g_bitmap_memory;
GLOBAL int         g_bitmap_width;
GLOBAL int         g_bitmap_height;
GLOBAL int         g_bytes_per_pixel = 4;

INTERNAL void RenderGradient(int x_offset, int y_offset) {
    int pitch = g_bitmap_width*g_bytes_per_pixel;
    u8 *row = (u8 *)g_bitmap_memory;
    for (int y = 0; y < g_bitmap_height; y++) { 
        u32 *pixel = (u32 *)row;
        for (int x = 0; x < g_bitmap_width; x++) { 

            u8 blue = x + x_offset;
            u8 green = y + y_offset;

            *pixel++ = ((green << 8) | blue);
        }

        row += pitch;
    }
}

INTERNAL void Win32ResizeDIBSection(int width, int height) {
    if (g_bitmap_memory) {
        VirtualFree(g_bitmap_memory, 0, MEM_RELEASE);
    }

    g_bitmap_width  = width;
    g_bitmap_height = height;

    g_bitmap_info.bmiHeader.biSize        = sizeof(g_bitmap_info.bmiHeader);
    g_bitmap_info.bmiHeader.biWidth       = g_bitmap_width;
    g_bitmap_info.bmiHeader.biHeight      = -g_bitmap_height;
    g_bitmap_info.bmiHeader.biPlanes      = 1;
    g_bitmap_info.bmiHeader.biBitCount    = 32;
    g_bitmap_info.bmiHeader.biCompression = BI_RGB;

    int bitmap_memory_size = g_bitmap_width * g_bitmap_height * g_bytes_per_pixel;
    g_bitmap_memory = VirtualAlloc(0, bitmap_memory_size, MEM_COMMIT, PAGE_READWRITE);
}

INTERNAL void Win32UpdateWindow(HDC device_context, RECT client_rect, int x, int y, int width, int height) { 
    int window_width  = client_rect.right  - client_rect.left;
    int window_height = client_rect.bottom - client_rect.top;
    StretchDIBits(device_context, /*x, y, width, height, x, y, width, height, */ 
                  0, 0, window_width, window_height, 0, 0, g_bitmap_width, g_bitmap_height, g_bitmap_memory, 
                  &g_bitmap_info, DIB_RGB_COLORS, SRCCOPY);
}

LRESULT CALLBACK Win32MainWindowCallback(HWND window, UINT message, WPARAM w_param, LPARAM l_param) {
    LRESULT result = 0;
    switch (message) {
        case WM_SIZE: {
            RECT client_rect;
            GetClientRect(window, &client_rect);
            int width  = client_rect.right  - client_rect.left;
            int height = client_rect.bottom - client_rect.top;
            Win32ResizeDIBSection(width, height);
        } break;
        case WM_DESTROY:     g_running = false;                      break; // TODO: Handle this as an error and maybe recreate the window.
        case WM_CLOSE:       g_running = false;                      break; // TODO: Handle this with a message to the user.
        case WM_ACTIVATEAPP: OutputDebugStringA("WM_ACTIVATEAPP\n"); break;
        case WM_PAINT: {  
            PAINTSTRUCT paint;
            HDC device_context = BeginPaint(window, &paint);
            int x = paint.rcPaint.left;
            int y = paint.rcPaint.top;
            int width = paint.rcPaint.right - paint.rcPaint.left;
            int height = paint.rcPaint.bottom - paint.rcPaint.top;

            RECT client_rect;
            GetClientRect(window, &client_rect);
            Win32UpdateWindow(device_context, client_rect, x, y, width, height);
            EndPaint(window, &paint);
        } break;
        default: result = DefWindowProc(window, message, w_param, l_param); break;
    }
    return result;
}

int CALLBACK WinMain(HINSTANCE instance, HINSTANCE prev_instance, LPSTR command_line, int show_command_line) {
    WNDCLASS window_class    = {};
    window_class.style       = CS_HREDRAW|CS_VREDRAW;
    window_class.lpfnWndProc = Win32MainWindowCallback;
    window_class.hInstance   = instance;
    // window_class.hIcon;
    window_class.lpszClassName = "RasterWindowClass";

    if (RegisterClass(&window_class)) { 
        HWND window = CreateWindowEx(0, window_class.lpszClassName, "Rasteriser", 
                                     WS_OVERLAPPEDWINDOW|WS_VISIBLE, CW_USEDEFAULT,
                                     CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, 
                                     0, 0, instance, 0);
        if (window) {
            int x_offset = 0;
            int y_offset = 0;
            g_running = true;
            while(g_running) {
                MSG message; 
                while (PeekMessage(&message, 0, 0, 0, PM_REMOVE)) {
                    if (message.message == WM_QUIT) { 
                        g_running = false;
                    } 

                    TranslateMessage(&message);
                    DispatchMessageA(&message);
                }

                RenderGradient(x_offset, y_offset);
                HDC device_context = GetDC(window);
                RECT client_rect;
                GetClientRect(window, &client_rect);
                int window_width  = client_rect.right - client_rect.left;
                int window_height = client_rect.bottom - client_rect.top;
                Win32UpdateWindow(device_context, client_rect, 0, 0, window_width, window_height);
                ReleaseDC(window, device_context);

                x_offset++;
                y_offset++;
            }
        } else {
            // TODO: Do some logging here for failed window handle.
        }
    } else {
            // TODO: Do some logging here for failed window class.
    }
}

