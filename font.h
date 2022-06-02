#ifndef FONT_H_
#define FONT_H_

#include <cstdint>
#include "stb_truetype.h"

struct point {
    int x, y;
};

struct glyph {
    uint8_t *bitmap;
    int stride;
    /* Bitmap width, height from origin to end */
    point dim;
    /* Offset from start of bitmap pointer to bitmap origin */
    point ptr_off;
    /* Offset from bitmap origin to bounding box origin */
    point box_off;

    uint8_t operator()(int x, int y);
};

class font {
    stbtt_fontinfo info;
    uint8_t *font_buf;
    float font_scale;
    int descent;
    glyph glyphs[256] = {};
public:
    int char_width, char_height;
    font(char *path, int scale);
    ~font();
    glyph &operator[](char c);
};

#endif // FONT_H_
