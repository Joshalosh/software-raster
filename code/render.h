
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

struct Game_Bitmap {
void *memory;
int  width;
int  height;
int  pitch;
int  bytes_per_pixel;
};

struct Game_Memory {
    B32   is_initialised;
    U64   persisting_storage_size;
    void *persisting_storage; // This is required to be cleared to zero at startup.
    U64   temporary_storage_size;
    void *temporary_storage;
};

INTERNAL void GameUpdateAndRender(Game_Memory *memory, Game_Bitmap *bitmap);
