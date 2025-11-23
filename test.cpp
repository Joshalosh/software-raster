
#include <windows.h>

#define GLOBAL        static
#define LOCAL_PERSIST static
#define INTERNAL      static

GLOBAL bool g_running;

INTERNAL void Win32ResizeDIBSection(int width, int height) {
}

LRESULT CALLBACK Win32MainWindowCallback(HWND window, UINT message, WPARAM w_param, LPARAM l_param) {
    LRESULT result = 0;
    switch (message) {
        case WM_SIZE: {
            RECT client_rect = GetClientRect(window, &client_rect);
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
        int x = paint.rcPaint.left;
        int y = paint.rcPaint.top;
        int width = paint.rcPaint.right - paint.rcPaint.left;
        int height = paint.rcPaint.bottom - paint.rcPaint.top;
        LOCAL_PERSIST DWORD operation = WHITENESS;
        PatBlt(device_context, x, y, width, height, operation);  
        if (operation == WHITENESS ) 
            operation = BLACKNESS;
        else  
            operation = WHITENESS;

        EndPaint(window, &paint);
        } break;

        default: result = DefWindowProc(window, message, w_param, l_param); break;
    }
    return result;
}


int CALLBACK WinMain(HINSTANCE instance, HINSTANCE prev_instance, LPSTR command_line, int show_command_line) {
    WNDCLASS window_class    = {};
    window_class.lpfnWndProc = Win32MainWindowCallback;
    window_class.hInstance   = instance;
    window_class.lpszClassName = "TestWindowClass";
    g_running = true;

    if (RegisterClassA(&window_class)) {
        HWND window_handle = CreateWindowExA(0, window_class.lpszClassName, "TestWindow", WS_OVERLAPPEDWINDOW|WS_VISIBLE,
                                            CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, 0, 0, instance, 0);
        if (window_handle) {
            while (g_running) {
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
            // TODO: failed window handle logging
        }
    } else { 
        // TODO: Failed window class logging
    }
    return 0;
}
