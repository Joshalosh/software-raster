
#include "render.h"

INTERNAL void RenderGradient(Game_Bitmap bitmap, int x_offset, int y_offset) {
    U8 *row = (U8 *)bitmap.memory;
    for (int y = 0; y < bitmap.height; y++) { 
        U32 *pixel = (U32 *)row;
        for (int x = 0; x < bitmap.width; x++) { 

            U8 blue = (U8)(x + x_offset);
            U8 green = (U8)(y + y_offset);

            *pixel++ = ((green << 8) | blue);
        }

        row += bitmap.pitch;
    }
}

INTERNAL S32 RoundR32ToS32(R32 a) {
    S32 result = (S32)(a + 0.5f);
    return result;
}

INTERNAL void DrawRectangle(Game_Bitmap *bitmap, V2 min, V2 max, U32 color) {
    S32 min_x = RoundR32ToS32(min.x);
    S32 min_y = RoundR32ToS32(min.y);
    S32 max_x = RoundR32ToS32(max.x);
    S32 max_y = RoundR32ToS32(max.y);

    if (min_x < 0)              min_x = 0;
    if (min_y < 0)              min_y = 0;
    if (max_x > bitmap->width)  max_x = bitmap->width;
    if (max_y > bitmap->height) max_y = bitmap->height;

    U8 *row = (U8 *)bitmap->memory + min_x*bitmap->bytes_per_pixel + min_y*bitmap->pitch;
    for (int y = min_y; y < max_y; y++) {
        U32 *pixel = (U32 *)row;
        for (int x = min_x; x < max_x; x++) { 
            *pixel++ = color;
        }
        row += bitmap->pitch;
    }
}

INTERNAL S32 SignedArea(V2 origin, V2 point1, V2 point2) {
    S32 result;
    V2 vector1 = point1 - origin;
    V2 vector2 = point2 - origin;
    result = (S32)((vector1.x*vector2.y) - (vector1.y*vector2.x));
    return result;
}

INTERNAL B32 IsLeftOrTopEdge(V2 origin_vertex, V2 next_vertex) {
    B32 result;
    V2 edge = next_vertex - origin_vertex;
    B32 is_left_edge = edge.y > 0;
    B32 is_top_edge  = (edge.y == 0) && (edge.x < 0);
    result = is_left_edge || is_top_edge;
    return result;
}

INTERNAL void DrawTriangle(Game_Bitmap *bitmap, V2 vert0, V2 vert1, V2 vert2, V4 col0, V4 col1, V4 col2) {
    S32 min_x = RoundR32ToS32(MIN(MIN(vert0.x, vert1.x), vert2.x));
    S32 min_y = RoundR32ToS32(MIN(MIN(vert0.y, vert1.y), vert2.y));
    S32 max_x = RoundR32ToS32(MAX(MAX(vert0.x, vert1.x), vert2.x));
    S32 max_y = RoundR32ToS32(MAX(MAX(vert0.y, vert1.y), vert2.y));

    if (min_x < 0)              min_x = 0;
    if (min_y < 0)              min_y = 0;
    if (max_x > bitmap->width)  max_x = bitmap->width;
    if (max_y > bitmap->height) max_y = bitmap->height;

    V2 edge0 = (vert2 - vert1);
    V2 edge1 = (vert0 - vert2);
    V2 edge2 = (vert1 - vert0);

    V2 norm0 = V2{-edge0.y, edge0.x};
    V2 norm1 = V2{-edge1.y, edge1.x};
    V2 norm2 = V2{-edge2.y, edge2.x};

    S32 w0_dx = (S32)norm0.x;
    S32 w1_dx = (S32)norm1.x;
    S32 w2_dx = (S32)norm2.x;

    S32 w0_dy = (S32)norm0.y;
    S32 w1_dy = (S32)norm1.y;
    S32 w2_dy = (S32)norm2.y;

    S32 bias0 = IsLeftOrTopEdge(vert1, vert2) ? 0 : 1;
    S32 bias1 = IsLeftOrTopEdge(vert2, vert0) ? 0 : 1;
    S32 bias2 = IsLeftOrTopEdge(vert0, vert1) ? 0 : 1;

    R32 triangle_area = (R32)SignedArea(vert0, vert1, vert2);
    R32 inverse_triangle_area = 1/triangle_area;

    V2 pixel_coord = {(R32)min_x, (R32)min_y};
    S32 w0_row_start = SignedArea(vert1, vert2, pixel_coord) + bias0;
    S32 w1_row_start = SignedArea(vert2, vert0, pixel_coord) + bias1;
    S32 w2_row_start = SignedArea(vert0, vert1, pixel_coord) + bias2;

    U8 *row = (U8 *)bitmap->memory + min_x*bitmap->bytes_per_pixel + min_y*bitmap->pitch;
    for (int y = min_y; y < max_y; y++) {
        S32 weight0 = w0_row_start;
        S32 weight1 = w1_row_start;
        S32 weight2 = w2_row_start;
        U32 *pixel = (U32 *)row;
        for (int x = min_x; x < max_x; x++) {
            if (weight0 <= 0 && weight1 <= 0 && weight2 <= 0) {
                R32 ratio0 = weight0 * inverse_triangle_area;
                R32 ratio1 = weight1 * inverse_triangle_area;
                R32 ratio2 = weight2 * inverse_triangle_area;

                R32 r = (ratio0*col0.r) + (ratio1*col1.r) + (ratio2*col2.r);
                R32 g = (ratio0*col0.g) + (ratio1*col1.g) + (ratio2*col2.g);
                R32 b = (ratio0*col0.b) + (ratio1*col1.b) + (ratio2*col2.b);
                R32 a = (ratio0*col0.a) + (ratio1*col1.a) + (ratio2*col2.a);

                U8 red   = (U8)((r * 255.0f) + 0.5f);
                U8 green = (U8)((g * 255.0f) + 0.5f);
                U8 blue  = (U8)((b * 255.0f) + 0.5f);
                U8 alpha = (U8)((a * 255.0f) + 0.5f);

                U32 col = ((alpha << 24) | (red << 16) | (green << 8) | blue);

                *pixel = col;
            }
            weight0 += w0_dx;
            weight1 += w1_dx;
            weight2 += w2_dx;
            pixel++;
        }
        w0_row_start += w0_dy;
        w1_row_start += w1_dy;
        w2_row_start += w2_dy;
        row += bitmap->pitch;
    }
}

#pragma pack(push, 1)
struct Bitmap_Header {
    U16 file_type;
    U32 file_size;
    U16 reserved_1;
    U16 reserved_2;
    U32 bitmap_offset;
    U32 size;
    S32 width;
    S32 height;
    U16 planes;
    U16 bits_per_pixel;
    U32 compression;
    U32 image_size;
    U32 pixels_per_metre_x;
    U32 pixels_per_metre_y;
    U32 cols_used;
    U32 cols_important;
    U32 red_mask;
    U32 green_mask;
    U32 blue_mask;
    U32 alpha_mask;
};
#pragma pack(pop)

INTERNAL U32 *DEBUGLoadBMP(Debug_Platform_Read_Entire_File *ReadEntireFile, char *filename) {
    U32 *result = 0;

    Debug_Read_File_Result read_result = ReadEntireFile(filename);
    if (read_result.content_size != 0) {
        Bitmap_Header *header = (Bitmap_Header *)read_result.content;
        U32 *pixels = (U32 *)((U8 *)read_result.content + header->bitmap_offset);
        result = pixels;
    }

    return result;
}

INTERNAL U32 *TestLoadBMP(Debug_Platform_Read_Entire_File *ReadEntireFile, char *filename_1, char *filename_2) {
    U32 *result_1 = 0;
    U32 *result_2 = 0;

    Debug_Read_File_Result read_result_1 = ReadEntireFile(filename_1);
    Debug_Read_File_Result read_result_2 = ReadEntireFile(filename_2);
    if ((read_result_1.content_size != 0) && (read_result_2.content_size != 0)) {
        Bitmap_Header *header_1 = (Bitmap_Header *)read_result_1.content;
        Bitmap_Header *header_2 = (Bitmap_Header *)read_result_2.content;
        U32 *pixels_1 = (U32 *)((U8 *)read_result_1.content + header_1->bitmap_offset);
        U32 *pixels_2 = (U32 *)((U8 *)read_result_2.content + header_2->bitmap_offset);
        result_1 = pixels_1;
        result_2 = pixels_2;
    }

    return result_1;
}
extern "C" GAME_UPDATE_AND_RENDER(GameUpdateAndRender) {
    Game_State *game_state = (Game_State *)memory->persisting_storage;
    if (!memory->is_initialised) {
#if 0
        char *filename = __FILE__;
        Debug_Read_File_Result file = memory->DEBUGPlatformReadEntireFile(filename);
        if (file.content) {
            memory->DEBUGPlatformWriteEntireFile("test.out", file.content_size, file.content);
            memory->DEBUGPlatformFreeFileMemory(file.content);
        }
#endif
        char *bmp_1 = "art.bmp";
        char *bmp_2 = "test_background.bmp";
        char *bmp_3 = "gimp.bmp";
        game_state->pixel_ptr = TestLoadBMP(memory->DEBUGPlatformReadEntireFile, bmp_1, bmp_2);
        //game_state->pixel_ptr = DEBUGLoadBMP(memory->DEBUGPlatformReadEntireFile, bmp_3);

        memory->is_initialised = true;
    }
    //RenderGradient(bitmap, x_offset, y_offset);
    U32 purple = 0x00FF0000;
    U32 white  = 0x00FFFFFF;
    V4  col0   = {0.03f, 0.52f, 0.63f, 1.0f};
    //V4  col0   = {0.0f, 0.0f, 1.0f, 1.0f};
    V4  col1   = {0.46f, 0.12f, 0.62f, 1.0f};
    V4  col2   = {0.63f, 0.82f, 0.05f, 1.0f};
    V4  red    = {0.83f, 0.93f, 0.49f, 1.0f};
    V4  green  = {0.85f, 0.7f,  0.05f, 1.0f};
    V4  blue   = {0.06f, 0.50f, 0.80f, 1.0f};

    DrawRectangle(bitmap, V2{0.0f, 0.0f}, V2{(R32)bitmap->width, (R32)bitmap->height}, purple);
    //DrawRectangle(bitmap, V2{20.0f, 20.0f}, V2{21.0f, 21.0f}, 0x0000FF00);
    DrawTriangle(bitmap, V2{50, 300},  V2{150, 300}, V2{100, 100}, col0, col1, col2);
    DrawTriangle(bitmap, V2{250, 225}, V2{100, 100}, V2{150, 300}, red, green, blue);
    DrawTriangle(bitmap, V2{450, 425}, V2{400, 400}, V2{350, 500}, red, green, blue);

#if 0
    S32 pixel_width = 60;
    S32 pixel_height = 40;

    S32 blit_width = pixel_width;
    S32 blit_height = pixel_height;
    if (blit_width > bitmap->width) blit_width = bitmap->width;
    if (blit_height > bitmap->height) blit_height = bitmap->height;

    U32 *src = game_state->pixel_ptr;
    U8 *dest_row = (U8 *)bitmap->memory;
    for (S32 y = 0; y < pixel_height; y++) {
        U32 *dest = (U32 *)dest_row;
        for (S32 x = 0; x < pixel_width; x++) {
            *dest++ = *src++;
        }
        dest_row += bitmap->pitch;
    }
#endif
}
