#include "font.h"
#include <stdio.h>
#include <sys/stat.h>
#include <algorithm>
#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"

glyph &font::operator[](char c) {
    /* Bitmaps are lazily loaded */
    glyph &g = glyphs[(int)c];
    if (g.bitmap != nullptr) {
        return g;
    }

    int baseline = char_height + descent;
    int w, h, xoff, yoff;
    g.bitmap = stbtt_GetCodepointBitmapSubpixel(&info, font_scale, font_scale, 0, 0, c,
                                                &w, &h, &xoff, &yoff);
    g.stride = w;

    /* Ignore offsets that go beyond the top or left */
    g.box_off.x = std::max(xoff, 0);
    g.box_off.y = std::max(baseline + yoff, 0);

    /* Skip ahead if glyph extends past the bounding box to the left or top */
    g.ptr_off.x = -std::min(xoff, 0);
    g.ptr_off.y = -std::min(baseline + yoff, 0);

    /* Limit dimensions if glyph extends past the bounding box to the bottom or right */
    g.dim.x = std::min(w - g.ptr_off.x, char_width - g.box_off.x);
    g.dim.y = std::min(h - g.ptr_off.y, char_height - g.box_off.y);

    return g;
}

uint8_t glyph::operator()(int x, int y) {
    int i = (y + ptr_off.y) * stride + (ptr_off.x + x);
    return bitmap[i];
}

font::font(char *path, int scale) {
    struct stat st;
    if (stat(path, &st) < 0) {
        perror("stat");
        exit(1);
    }
    FILE *fp = fopen(path, "rb");
    if (fp == NULL) {
        perror("fopen");
        exit(1);
    }
    font_buf = new uint8_t[st.st_size];
    fread(font_buf, st.st_size, 1, fp);
    fclose(fp);

    stbtt_InitFont(&info, font_buf, stbtt_GetFontOffsetForIndex(font_buf, 0));

    font_scale = stbtt_ScaleForPixelHeight(&info, scale);

    /* Add padding for out of bounds glyphs */
    int ascent;
    stbtt_GetFontVMetrics(&info, &ascent, &descent, 0);
    char_height = (ascent - descent) * font_scale;
    descent *= font_scale;

    /* All character widths are equal in monospace fonts */
    stbtt_GetCodepointHMetrics(&info, ' ', &char_width, 0);
    char_width *= font_scale;
}

font::~font() {
    free(font_buf);
    for (glyph &g : glyphs) {
        free(g.bitmap);
    }
}
