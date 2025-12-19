
#include <stdint.h>

#define GLOBAL        static // Explicit for global variables
#define LOCAL_PERSIST static // Explicit for locally persisting variables
#define INTERNAL      static // Explicit for functions internal to the translation unit

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

#include "mymath.h"

#define KILOBYTES(value) ((value)*1024LL)
#define MEGABYTES(value) (KILOBYTES(value)*1024LL)
#define GIGABYTES(value) (MEGABYTES(value)*1024LL)
#define TERABYTES(value) (GIGABYTES(value)*1024LL)

#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))

#if GAME_SLOW
#define ASSERT(expression) if(!(expression)) {*(int *)0 = 0;}
#else 
#define ASSERT(expression)
#endif

INTERNAL U32 SafeU64ToU32(U64 input) {
    ASSERT(input <= 0xFFFFFFFF)
    U32 result = (U32)input;
    return result;
}

// Services that the platform layer provides to the game
#if GAME_INTERNAL
struct Debug_Read_File_Result {
    U32   content_size;
    void *content;
};
#define DEBUG_PLATFORM_FREE_FILE_MEMORY(name) void name(void *memory)
typedef DEBUG_PLATFORM_FREE_FILE_MEMORY(Debug_Platform_Free_File_Memory);

#define DEBUG_PLATFORM_READ_ENTIRE_FILE(name) Debug_Read_File_Result name(char *filename)
typedef DEBUG_PLATFORM_READ_ENTIRE_FILE(Debug_Platform_Read_Entire_File);

#define DEBUG_PLATFORM_WRITE_ENTIRE_FILE(name) B32 name(char *filename, U32 memory_size, void *memory)
typedef DEBUG_PLATFORM_WRITE_ENTIRE_FILE(Debug_Platform_Write_Entire_File);
#endif
// Services that the game provides to the platform layer

struct Game_Bitmap {
void *memory;
int  width;
int  height;
int  pitch;
int  bytes_per_pixel;
};

struct Loaded_Bitmap {
    S32 width;
    S32 height;
    S32 pitch;
    U32 pixels[16024];
};

struct Game_State {
    U32 *pixel_ptr;
    Loaded_Bitmap loaded_bitmap;
};

struct Game_Memory {
    B32   is_initialised;
    U64   persisting_storage_size;
    void *persisting_storage; // This is required to be cleared to zero at startup.
    U64   temporary_storage_size;
    void *temporary_storage;

    Debug_Platform_Free_File_Memory  *DEBUGPlatformFreeFileMemory;
    Debug_Platform_Read_Entire_File  *DEBUGPlatformReadEntireFile;
    Debug_Platform_Write_Entire_File *DEBUGPlatformWriteEntireFile;
};

#define GAME_UPDATE_AND_RENDER(name) void name(Game_Memory *memory, Game_Bitmap *bitmap)
typedef GAME_UPDATE_AND_RENDER(Game_Update_And_Render);
