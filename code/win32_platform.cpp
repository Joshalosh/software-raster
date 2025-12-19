

#include "render.h"

#include <windows.h>
#include <stdio.h>

#include "win32_platform.h"

GLOBAL bool         g_running;
GLOBAL Win32_Bitmap g_bitmap;
GLOBAL S64          g_tick_frequency;

Win32_Window_Dimension Win32GetWindowDimension(HWND window) {
    Win32_Window_Dimension result;
    RECT client_rect;
    GetClientRect(window, &client_rect);
    result.width = client_rect.right - client_rect.left;
    result.height = client_rect.bottom - client_rect.top;
    return result;
}

INTERNAL void Win32ResizeDIBSection(Win32_Bitmap *bitmap, int width, int height) {
    S32 bytes_per_pixel = 4;
    S32 bitmap_memory_size = width * height * bytes_per_pixel;
    void *new_memory = VirtualAlloc(0, bitmap_memory_size, MEM_COMMIT, PAGE_READWRITE);
    if (new_memory) {
        if (bitmap->memory) {
            VirtualFree(bitmap->memory, 0, MEM_RELEASE);
        }

        bitmap->width  = width;
        bitmap->height = height;
        bitmap->bytes_per_pixel = bytes_per_pixel;

        bitmap->info.bmiHeader.biSize        = sizeof(bitmap->info.bmiHeader);
        bitmap->info.bmiHeader.biWidth       = bitmap->width;
        bitmap->info.bmiHeader.biHeight      = -bitmap->height;
        bitmap->info.bmiHeader.biPlanes      = 1;
        bitmap->info.bmiHeader.biBitCount    = 32;
        bitmap->info.bmiHeader.biCompression = BI_RGB;

        bitmap->memory = new_memory;
        bitmap->pitch  = bitmap->width * bitmap->bytes_per_pixel;
    }
}

INTERNAL void Win32CopyBitmapToWindow(HDC device_context, Win32_Bitmap bitmap, 
                                      int window_width, int window_height) { 
    StretchDIBits(device_context, 
                  0, 0, window_width, window_height, 0, 0, bitmap.width, bitmap.height, bitmap.memory, 
                  &bitmap.info, DIB_RGB_COLORS, SRCCOPY);
}

DEBUG_PLATFORM_FREE_FILE_MEMORY(DEBUGPlatformFreeFileMemory) {
    if (memory) {
        VirtualFree(memory, 0, MEM_RELEASE);
    }
}

DEBUG_PLATFORM_READ_ENTIRE_FILE(DEBUGPlatformReadEntireFile) {
    Debug_Read_File_Result result = {};
    HANDLE file_handle = CreateFileA(filename, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);
    if (file_handle != INVALID_HANDLE_VALUE) {
        LARGE_INTEGER file_size;
        if (GetFileSizeEx(file_handle, &file_size)) {
            U32 file_size32 = SafeU64ToU32(file_size.QuadPart);
            result.content = VirtualAlloc(0, file_size32, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
            if (result.content) {
                DWORD bytes_read;
                if (ReadFile(file_handle, result.content, file_size32, &bytes_read, 0) && (file_size32 == bytes_read)) {
                    // The file was read succesfully.
                    result.content_size = file_size32;
                } else {
                    // TODO: Log that the file wasn't successfully read.
                    DEBUGPlatformFreeFileMemory(result.content);
                    result.content = 0;
                }
            } else {
                // TODO: Log that memory allocation has failed.
            }
        } else {
            // TODO: Log that the handle file size was not extracted successfully.
        }

        CloseHandle(file_handle);
    } else {
        // TODO: Log the file handle was not created successfully.
    }
    return result;
}

DEBUG_PLATFORM_WRITE_ENTIRE_FILE(DEBUGPlatformWriteEntireFile) {
    B32 result = false;

    HANDLE file_handle = CreateFileA(filename, GENERIC_WRITE, 0, 0, CREATE_ALWAYS, 0, 0);
    if (file_handle != INVALID_HANDLE_VALUE) {
        DWORD bytes_written;  
        if (WriteFile(file_handle, memory, memory_size, &bytes_written, 0)) {
            result = (bytes_written == memory_size);
        } else {
            // TODO: Log that file write failed.
        }

        CloseHandle(file_handle);
    } else {
        // TODO: Log the file handle was not created successfully.
    }
    return result;
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

INTERNAL FILETIME Win32GetLastWriteTime(char *filename) {
    FILETIME last_write_time = {};
    WIN32_FILE_ATTRIBUTE_DATA data;
    if (GetFileAttributesExA(filename, GetFileExInfoStandard, &data)) {
        last_write_time = data.ftLastWriteTime;
    }
    return last_write_time;
}

INTERNAL Win32_Game_Code Win32LoadGameCode(char *source_dll_path, char *temp_dll_path) {
    Win32_Game_Code result = {};
    result.last_dll_write_time = Win32GetLastWriteTime(source_dll_path);
    if (CopyFile(source_dll_path, temp_dll_path, FALSE)) { 
        result.game_code_dll = LoadLibraryA(temp_dll_path);
        if (result.game_code_dll) {
            result.update_and_render = (Game_Update_And_Render *)
                                       GetProcAddress(result.game_code_dll, "GameUpdateAndRender");
            result.is_valid = (result.update_and_render != 0);
        }
    } else {
        // TODO: The copy failed, log dat shit.
    }
    if (!result.is_valid) {
        result.update_and_render = 0;
    }
    return result;
}

INTERNAL void Win32UnloadGameCode(Win32_Game_Code *game_code) {
    if (game_code->game_code_dll) {     
        FreeLibrary(game_code->game_code_dll);
        game_code->game_code_dll = 0;
    }

    game_code->is_valid = false;
    game_code->update_and_render = 0;
}

INTERNAL void ConcatenateStrings(size_t source_a_count, char *source_a, size_t source_b_count, char *source_b, 
                                 size_t dest_count, char *dest) {
    ASSERT(source_a_count + source_b_count < dest_count);
    for (S32 index = 0; index < source_a_count; index++) {
        *dest++ = *source_a++;
    }
    for (S32 index = 0; index < source_b_count; index++) {
        *dest++ = *source_b++;
    }
    *dest = 0;
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

#if GAME_INTERNAL 
            LPVOID base_address = (LPVOID)TERABYTES(2);
#else 
            LPVOID base_address = 0;
#endif
            Game_Memory game_memory = {};
            game_memory.persisting_storage_size      = MEGABYTES(64);
            game_memory.temporary_storage_size       = GIGABYTES(4);
            game_memory.DEBUGPlatformFreeFileMemory  = DEBUGPlatformFreeFileMemory;
            game_memory.DEBUGPlatformReadEntireFile  = DEBUGPlatformReadEntireFile;
            game_memory.DEBUGPlatformWriteEntireFile = DEBUGPlatformWriteEntireFile;

            U64 total_memory_size               = game_memory.persisting_storage_size + 
                                                  game_memory.temporary_storage_size;
            game_memory.persisting_storage      = VirtualAlloc(base_address, total_memory_size, 
                                                               MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
            game_memory.temporary_storage       = (U8 *)game_memory.persisting_storage + 
                                                        game_memory.persisting_storage_size;
            if (game_memory.persisting_storage && game_memory.temporary_storage) {
                g_running = true;

                char exe_filename[MAX_PATH]; // Max path is a little bit dodgey.
                DWORD size_of_filename = GetModuleFileNameA(0, exe_filename, sizeof(exe_filename));
                // TODO: one_past_last_slash needs a better backup solution if GetModuleFileNameA fails.
                if (size_of_filename == 0 || size_of_filename == sizeof(exe_filename)) {
                    // TODO: Log error that the executable couldn't be found or the path was too long
                    return 0;
                }

                char *one_past_last_slash = exe_filename;
                for (char *scan = exe_filename; *scan; scan++) {
                    if (*scan == '\\') {
                        one_past_last_slash = scan + 1;
                    }
                }

                char source_dll_name[] = "render.dll";
                size_t source_dll_name_length_without_null_terminator = sizeof(source_dll_name) - 1;
                char source_dll_full_path[MAX_PATH];
                size_t current_path_length = one_past_last_slash - exe_filename;
                ConcatenateStrings(current_path_length, exe_filename,
                                   source_dll_name_length_without_null_terminator, source_dll_name,
                                   sizeof(source_dll_full_path), source_dll_full_path);

                char temp_dll_name[] = "render_temp.dll";
                size_t temp_dll_name_length_without_null_terminator = sizeof(temp_dll_name) - 1;
                char temp_dll_full_path[MAX_PATH];
                ConcatenateStrings(current_path_length, exe_filename,
                                   temp_dll_name_length_without_null_terminator, temp_dll_name,
                                   sizeof(temp_dll_full_path), temp_dll_full_path);

                Win32_Game_Code game = Win32LoadGameCode(source_dll_full_path, temp_dll_full_path);
                LARGE_INTEGER begin_frame_count = Win32GetTickCount();
                U64 begin_cycle_count = __rdtsc();

                while(g_running) {
                    FILETIME new_dll_write_time = Win32GetLastWriteTime(source_dll_full_path);
                    if (CompareFileTime(&new_dll_write_time, &game.last_dll_write_time) != 0) {
                        Win32UnloadGameCode(&game);
                        for (U32 load_tries = 0; !game.is_valid && (load_tries < 100); load_tries++) {
                            game = Win32LoadGameCode(source_dll_full_path, temp_dll_full_path);
                            Sleep(100);
                        }
                    }

                    MSG message; 
                    while (PeekMessage(&message, 0, 0, 0, PM_REMOVE)) {
                        if (message.message == WM_QUIT) { 
                            g_running = false;
                        } 

                        TranslateMessage(&message);
                        DispatchMessageA(&message);
                    }
                    Game_Bitmap bitmap = {};
                    bitmap.memory          = g_bitmap.memory;
                    bitmap.width           = g_bitmap.width;
                    bitmap.height          = g_bitmap.height;
                    bitmap.pitch           = g_bitmap.pitch;
                    bitmap.bytes_per_pixel = g_bitmap.bytes_per_pixel;
                    if (game.update_and_render) {
                        game.update_and_render(&game_memory, &bitmap);
                    }

                    Win32_Window_Dimension dimension = Win32GetWindowDimension(window);
                    Win32CopyBitmapToWindow(device_context, g_bitmap, dimension.width, dimension.height);

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
                    R64 cycles_this_frame = (R64)(end_cycle_count - begin_cycle_count);
                    R32 mega_cycles_per_frame = (R32)(cycles_this_frame / (1000.0f*1000.0f));
                    R32 ms_per_frame = 1000.0f*frame_seconds_elapsed;

                    char buffer[256];
                    sprintf(buffer, "%.02fms,    %.02ffps,    %.02fcycles\n", ms_per_frame, fps, mega_cycles_per_frame);
                    OutputDebugStringA(buffer);

                    begin_frame_count = current_ticks;
                    begin_cycle_count = end_cycle_count;
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

