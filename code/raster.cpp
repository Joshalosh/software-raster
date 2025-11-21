
#include <windows.h>
#include <stdint.h>

#define GLOBAL        static // Explicit for global variables
#define LOCAL_PERSIST static // Explicit for locally persisting variables
#define INTERNAL      static // Explicit for functions internal to the translation unit

GLOBAL bool        running;
GLOBAL BITMAPINFO  bitmap_info;
GLOBAL void       *bitmap_memory;
GLOBAL HBITMAP     bitmap_handle;
GLOBAL HDC         bitmap_device_context;

INTERNAL void Win32ResizeDIBSection(int width, int height) {
    if (bitmap_handle) { 
        DeleteObject(bitmap_handle);
    }

    if (!bitmap_device_context) { 
        bitmap_device_context = CreateCompatibleDC(0);
    }

    bitmap_info.bmiHeader.biSize        = sizeof(bitmap_info.bmiHeader);
    bitmap_info.bmiHeader.biWidth       = width;
    bitmap_info.bmiHeader.biHeight      = height;
    bitmap_info.bmiHeader.biPlanes      = 1;
    bitmap_info.bmiHeader.biBitCount    = 32;
    bitmap_info.bmiHeader.biCompression = BI_RGB;

    bitmap_handle = CreateDIBSection(bitmap_device_context, &bitmap_info, DIB_RGB_COLORS, &bitmap_memory, 0, 0);
}

INTERNAL void Win32UpdateWindow(HDC device_context, int x, int y, int width, int height) { 
    StretchDIBits(device_context, x, y, width, height, x, y, width, height, bitmap_memory, 
                  &bitmap_info, DIB_RGB_COLORS, SRCCOPY);
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
        case WM_DESTROY:     running = false;                        break; // TODO: Handle this as an error and maybe recreate the window.
        case WM_CLOSE:       running = false;                        break; // TODO: Handle this with a message to the user.
        case WM_ACTIVATEAPP: OutputDebugStringA("WM_ACTIVATEAPP\n"); break;
        case WM_PAINT: {  
            PAINTSTRUCT paint;
            HDC device_context = BeginPaint(window, &paint);
            int x = paint.rcPaint.left;
            int y = paint.rcPaint.top;
            int width = paint.rcPaint.right - paint.rcPaint.left;
            int height = paint.rcPaint.bottom - paint.rcPaint.top;
            Win32UpdateWindow(device_context, x, y, width, height);
            EndPaint(window, &paint);
        } break;
        default: result = DefWindowProc(window, message, w_param, l_param); break;
    }
    return result;
}

int CALLBACK WinMain(HINSTANCE instance, HINSTANCE prev_instance, LPSTR command_line, int show_code) {
    WNDCLASS window_class = {};
    window_class.lpfnWndProc = Win32MainWindowCallback;
    window_class.hInstance = instance;
    // window_class.hIcon;
    window_class.lpszClassName = "RasterWindowClass";

    if (RegisterClass(&window_class)) { 
        HWND window_handle = CreateWindowEx(0, window_class.lpszClassName, "Rasteriser", 
                                            WS_OVERLAPPEDWINDOW|WS_VISIBLE, CW_USEDEFAULT,
                                            CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, 
                                            0, 0, instance, 0);
        if (window_handle) {
            for(;;) {
                MSG message; 
                BOOL message_result = GetMessage(&message, 0, 0, 0);
                if (message_result > 0) {
                    TranslateMessage(&message);
                    DispatchMessage(&message);
                } else {
                    break;
                }
            }
        } else {
            // TODO: Do some logging here for failed window handle.
        }
    } else {
            // TODO: Do some logging here for failed window class.
    }
    return 0;
}

