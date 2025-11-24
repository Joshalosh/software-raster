
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

struct Win32_Bitmap {
BITMAPINFO  info;
void       *memory;
int         width;
int         height;
int         pitch;
};

GLOBAL bool         g_running;
GLOBAL Win32_Bitmap g_bitmap;

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
    u8 *row = (u8 *)bitmap.memory;
    for (int y = 0; y < bitmap.height; y++) { 
        u32 *pixel = (u32 *)row;
        for (int x = 0; x < bitmap.width; x++) { 

            u8 blue = x + x_offset;
            u8 green = y + y_offset;

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
    int bytes_per_pixel = 4;

    bitmap->info.bmiHeader.biSize        = sizeof(bitmap->info.bmiHeader);
    bitmap->info.bmiHeader.biWidth       = bitmap->width;
    bitmap->info.bmiHeader.biHeight      = -bitmap->height;
    bitmap->info.bmiHeader.biPlanes      = 1;
    bitmap->info.bmiHeader.biBitCount    = 32;
    bitmap->info.bmiHeader.biCompression = BI_RGB;

    int bitmap_memory_size = bitmap->width * bitmap->height * bytes_per_pixel;
    bitmap->memory = VirtualAlloc(0, bitmap_memory_size, MEM_COMMIT, PAGE_READWRITE);
    bitmap->pitch  = bitmap->width * bytes_per_pixel;
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

int CALLBACK WinMain(HINSTANCE instance, HINSTANCE prev_instance, LPSTR command_line, int show_command_line) {
    WNDCLASS window_class    = {};
    Win32ResizeDIBSection(&g_bitmap, 1280, 720);
    window_class.style       = CS_HREDRAW|CS_VREDRAW|CS_OWNDC;
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
            HDC device_context = GetDC(window);
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
                RenderGradient(g_bitmap, x_offset, y_offset);
                Win32_Window_Dimension dimension = Win32GetWindowDimension(window);
                Win32CopyBitmapToWindow(device_context, g_bitmap, dimension.width, dimension.height);
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

