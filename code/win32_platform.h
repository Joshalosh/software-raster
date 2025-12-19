
struct Win32_Bitmap {
BITMAPINFO  info;
void       *memory;
int         width;
int         height;
int         pitch;
int         bytes_per_pixel;
};

struct Win32_Window_Dimension {
    int width;
    int height;
};

struct Win32_Game_Code {
    HMODULE game_code_dll;
    FILETIME last_dll_write_time;

    // The callback here can be null so make sure to check 
    // before calling.
    Game_Update_And_Render *update_and_render;
    B32 is_valid;
};
