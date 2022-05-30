#ifndef LCD_H_
#define LCD_H_

#include <cstdint>
#include <string>
#include "stb_truetype.h"

#define BAUDRATE B115200
#define CLEAR          102
#define SET_BRIGHTNESS 110
#define DISPLAY_BITMAP 197

#define WIDTH  320
#define HEIGHT 480

class lcd {
    int fd;
    stbtt_fontinfo font;
    uint8_t *font_buf;
    float scale;
    int char_width, char_height, descent, xoff;
    uint8_t screen[HEIGHT][WIDTH] = {};

    void send_command(uint8_t cmd, int x, int y, int ex, int ey);
    uint16_t pack_rgb(uint8_t signal, float r, float g, float b);
    void rasterize_char(char c, int bottom, int col);
public:
    void init_lcd(char *dev);
    void init_font(char *path);
    void write_line(std::string text, int line);
    void write_char(char c, int line, int col);
};

#endif // LCD_H_
