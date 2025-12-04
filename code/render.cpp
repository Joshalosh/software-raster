
#include "render.h"

INTERNAL void RenderGradient(Game_Bitmap bitmap, int x_offset, int y_offset) {
    U8 *row = (U8 *)bitmap.memory;
    for (int y = 0; y < bitmap.height; y++) { 
        U32 *pixel = (U32 *)row;
        for (int x = 0; x < bitmap.width; x++) { 

            U8 blue = x + x_offset;
            U8 green = y + y_offset;

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
    result = (vector1.x*vector2.y) - (vector1.y*vector2.x);
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
                R32 ratio0 = weight0 / triangle_area;
                R32 ratio1 = weight1 / triangle_area;
                R32 ratio2 = weight2 / triangle_area;

                R32 r = (ratio0*col0.r) + (ratio1*col1.r) + (ratio2*col2.r);
                R32 g = (ratio0*col0.g) + (ratio1*col1.g) + (ratio2*col2.g);
                R32 b = (ratio0*col0.b) + (ratio1*col1.b) + (ratio2*col2.b);

                U8 red   = r * 255.0f;
                U8 green = g * 255.0f;
                U8 blue  = b * 255.0f;

                U32 col = ((red << 16) | (green << 8) | blue);

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


INTERNAL void GameUpdateAndRender(Game_Memory *memory, Game_Bitmap *bitmap) {
    if (!memory->is_initialised) {
        memory->is_initialised = true;
    }
    //RenderGradient(bitmap, x_offset, y_offset);
    U32 purple = 0x00FF00FF;
    U32 white  = 0xFFFFFFFF;
    V4  col0   = {0.03f, 0.52f, 0.63f, 1.0f};
    V4  col1   = {0.46f, 0.12f, 0.62f, 1.0f};
    V4  col2   = {0.63f, 0.82f, 0.05f, 1.0f};
    V4  red    = {0.83f, 0.93f, 0.49f, 1.0f};
    V4  green  = {0.85f, 0.7f,  0.05f, 1.0f};
    V4  blue   = {0.06f, 0.50f, 0.80f, 1.0f};

    DrawRectangle(bitmap, V2{0.0f, 0.0f}, V2{(R32)bitmap->width, (R32)bitmap->height}, purple);
    //DrawRectangle(bitmap, V2{20.0f, 20.0f}, V2{21.0f, 21.0f}, 0x0000FF00);
    DrawTriangle(bitmap, V2{100, 600}, V2{300, 600}, V2{200, 200}, col0, col1, col2);
    DrawTriangle(bitmap, V2{500, 450}, V2{200, 200}, V2{300, 600}, red, green, blue);
}
