
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
}

int CALLBACK WinMain(HINSTANCE instance, HINSTANCE prev_instance, LPSTR command_line, int show_code) {
    WNDCLASS window_class = {};
    window_class.style = CS_OWNDC|CS_HREDRAW|CS_VREDRAW;
    window_class.lpfnWndProc = MainWindowCallback;
    window_class.hinstance = instance;
    // window_class.hIcon;
    window_class.lpszClassName = "RasterWindowClass";
    return 0;
}

