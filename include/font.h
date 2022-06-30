#ifndef FONT_H_
#define FONT_H_

#include <cstdint>
#include <string>
#include <functional>
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

    struct Iterator {
        using iterator_category = std::input_iterator_tag;
        using difference_type   = std::ptrdiff_t;
        using value_type        = std::pair<point, uint8_t>;
        using pointer           = std::pair<point, uint8_t>*;
        using reference         = std::pair<point, uint8_t>&;

        Iterator(glyph &glyph) : g(glyph) {
            pos.x = 0;
            pos.y = 0;
        };

        value_type operator*() const {
            int i = (pos.y + g.ptr_off.y) * g.stride + (g.ptr_off.x + pos.x);
            point p = {pos.x + g.box_off.x, pos.y + g.box_off.y};
            return std::make_pair(p, g.bitmap[i]);
        }

        void operator++() {
            pos.x++;
            if (pos.x == g.dim.x) {
                pos.x = 0;
                pos.y++;
            }
        }

        /* Check if current iterator is done */
        friend bool operator!= (const Iterator& a, const Iterator& b) {
            return a.pos.y < a.g.dim.y;
        };

    private:
        point pos;
        glyph &g;
    };

    Iterator begin() {
        return Iterator(*this);
    }
    Iterator end() {
        return Iterator(*this);
    };
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
