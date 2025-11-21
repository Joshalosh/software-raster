
#include <windows.h>
#include <stdint.h>

#if 0
void* buffer_memory;
int buffer_width;
int buffer_height;
BITMAPINFO bitmap_info;

void ResizeDIBSection(int width, int height) {
    if (buffer_memory) VirtualFree(buffer_memory, 0, MEM_RELEASE);

    buffer_width = width;
    buffer_height = height;

    bitmap_info.bmiHeader.biSize = sizeof(bitmap_info.bmiHeader);
    bitmap_info.bmiHeader.biWidth = width;
    bitmap_info.bmiHeader.biHeight = -height;
    bitmap_info.bmiHeader.biPlanes = 1;
    bitmap_info.bmiHeader.biBitCount = 32;
    bitmap_info.bmiHeader.biCompression = BI_RGB;

    int bytes_per_pixel = 4;
    int bitmap_memory_size = (width * height) * bytes_per_pixel;

    buffer_memory = VirtualAlloc(0, bitmap_memory_size, MEM_COMMIT, PAGE_READWRITE);
}
#endif

LRESULT CALLBACK MainWindowCallback(HWND window, UINT message, WPARAM w_param, LPARAM l_param) {
    LRESULT result = 0;
    switch (message) {
        case WM_SIZE:        OutputDebugStringA("WM_SIZE\n");               break;
        case WM_DESTROY:     OutputDebugStringA("WM_DESTROY\n");            break;
        case WM_CLOSE:       OutputDebugStringA("WM_CLOSE\n");              break;
        case WM_ACTIVATEAPP: OutputDebugStringA("WM_ACTIVATEAPP\n");        break;
        case WM_PAINT: {  
            PAINTSTRUCT paint;
            HDC device_context = BeginPaint(window, &paint);
            int x = paint.rcPaint.left;
            int y = paint.rcPaint.top;
            int width = paint.rcPaint.right - paint.rcPaint.left;
            int height = paint.rcPaint.bottom - paint.rcPaint.top;
            static DWORD operation = WHITENESS;
            PatBlt(device_context, x, y, width, height, operation);
            if (operation == WHITENESS) {
                operation = BLACKNESS;
            } else { 
                operation = WHITENESS;
            }
            EndPaint(window, &paint);
        } break;
        default: result = DefWindowProc(window, message, w_param, l_param); break;
    }
    return result;
}

int CALLBACK WinMain(HINSTANCE instance, HINSTANCE prev_instance, LPSTR command_line, int show_code) {
    WNDCLASS window_class = {};
    window_class.lpfnWndProc = MainWindowCallback;
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

