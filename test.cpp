
#include <windows.h>
#include <stdint.h>

#define GLOBAL        static
#define LOCAL_PERSIST static
#define INTERNAL      static

GLOBAL bool g_running;
GLOBAL void *g_bitmap_memory;
GLOBAL BITMAPINFO g_bitmap_info;
GLOBAL int g_bitmap_width;
GLOBAL int g_bitmap_height;
GLOBAL int g_bytes_per_pixel = 4;

typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

INTERNAL void RenderGradient(int x_offset, int y_offset) {
    int pitch = g_bytes_per_pixel * g_bitmap_width;
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

    g_bitmap_width = width;
    g_bitmap_height = height;

    g_bitmap_info.bmiHeader.biSize = sizeof(g_bitmap_info.bmiHeader); 
    g_bitmap_info.bmiHeader.biWidth = g_bitmap_width; 
    g_bitmap_info.bmiHeader.biHeight = -g_bitmap_height; 
    g_bitmap_info.bmiHeader.biPlanes = 1; 
    g_bitmap_info.bmiHeader.biBitCount = 32; 
    g_bitmap_info.bmiHeader.biCompression = BI_RGB;

    int bitmap_memory_size = g_bitmap_width * g_bitmap_height * g_bytes_per_pixel;
    g_bitmap_memory = VirtualAlloc(0, bitmap_memory_size, MEM_COMMIT, PAGE_READWRITE);
}

INTERNAL void Win32UpdateWindow(HDC device_context, int dest_width, int dest_height) {
    StretchDIBits(device_context, 0, 0, dest_width, dest_height, 0, 0, g_bitmap_width, g_bitmap_height, g_bitmap_memory, 
                  &g_bitmap_info, DIB_RGB_COLORS, SRCCOPY);
}

LRESULT CALLBACK Win32MainWindowCallback(HWND window, UINT message, WPARAM w_param, LPARAM l_param) {
    LRESULT result = 0;
    switch (message) {
        case WM_SIZE: {
            RECT client_rect; 
            GetClientRect(window, &client_rect);
            int rect_width  = client_rect.right  - client_rect.left;
            int rect_height = client_rect.bottom - client_rect.top;
            Win32ResizeDIBSection(rect_width, rect_height);
        } break;
        case WM_CLOSE:   g_running = false; break;
        case WM_DESTROY: g_running = false; break;
        case WM_ACTIVATEAPP: OutputDebugStringA("WM_ACTIVATEAPP\n"); break;
        case WM_PAINT: {
        PAINTSTRUCT paint;
        HDC device_context = BeginPaint(window, &paint);
        RECT client_rect; 
        GetClientRect(window, &client_rect);
        int dest_width = client_rect.right - client_rect.left;
        int dest_height = client_rect.bottom - client_rect.top;
        Win32UpdateWindow(device_context, dest_width, dest_height);
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
    window_class.lpszClassName = "TestWindowClass";

    if (RegisterClassA(&window_class)) {
        HWND window_handle = CreateWindowExA(0, window_class.lpszClassName, "TestWindow", WS_OVERLAPPEDWINDOW|WS_VISIBLE,
                                            CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, 0, 0, instance, 0);
        if (window_handle) {
            int x_offset = 0;
            int y_offset = 0;
            g_running = true;
            while (g_running) {
                MSG message;
                while (PeekMessage(&message, 0, 0, 0, PM_REMOVE)) {
                    if (message.message == WM_QUIT) {
                        g_running = false;
                    }
                    TranslateMessage(&message);
                    DispatchMessage(&message);
                }

                RenderGradient(x_offset, y_offset);
                HDC device_context = GetDC(window_handle);
                RECT client_rect; 
                GetClientRect(window_handle, &client_rect);
                int window_width = client_rect.right - client_rect.left;
                int window_height = client_rect.bottom - client_rect.top;
                Win32UpdateWindow(device_context, window_width, window_height);
                x_offset++;
                y_offset++;
            }
        } else {
            // TODO: failed window handle logging
        }
    } else { 
        // TODO: Failed window class logging
    }
    return 0;
}
