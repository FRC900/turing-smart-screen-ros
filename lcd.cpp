#include "lcd.h"
#include <termios.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <string.h>
#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"

void lcd::send_command(uint8_t cmd, int x, int y, int ex, int ey) {
    uint8_t buf[6] = {0};
    buf[0] = x >> 2;
    buf[1] = ((x &  3) << 6) | (y >> 4);
    buf[2] = ((y & 15) << 4) | (ex >> 6);
    buf[3] = ((ex & 63) << 2) | (ey >> 8);
    buf[4] = ey & 255;
    buf[5] = cmd;
    write(fd, buf, 6);
}

uint16_t lcd::pack_rgb(uint8_t signal, float r, float g, float b) {
    /* Linear interpolation: f(0) = (1,1,1), f(1) = (r, g, b) */
    float x = (float) signal / 0xFF;
    r = (1 - x) + (x * r);
    g = (1 - x) + (x * g);
    b = (1 - x) + (x * b);
    uint8_t r1 = (int)(r * 0xFF) >> 3;
    uint8_t g1 = (int)(g * 0xFF) >> 2;
    uint8_t b1 = (int)(b * 0xFF) >> 3;
    return (r1 << 11) | (g1 << 5) | b1;
}

void lcd::rasterize_char(char c, int bottom, int col) {
    if (col >= (WIDTH - xoff) / char_width) {
        fprintf(stderr, "column is beyond side of display\n");
        exit(1);
    }
    if (bottom > HEIGHT) {
        fprintf(stderr, "line is beyond bottom of display\n");
        exit(1);
    }
    int xpos = xoff + (col * char_width);
    int x0, y0, x1, y1;
    stbtt_GetCodepointBitmapBoxSubpixel(&font, c, scale, scale,
                                        0, 0, &x0, &y0, &x1, &y1);
    int x = xpos + x0;
    int baseline = bottom + y0 + descent;
    if (x < 0 || baseline < 0) {
        fprintf(stderr, "glyph %c does not fit in bounding box\n", c);
        exit(1);
    }

    stbtt_MakeCodepointBitmapSubpixel(&font, &screen[baseline][x], x1 - x0, y1 - y0,
                                      WIDTH, scale, scale, 0, 0, c);
}

void lcd::init_lcd(char *dev) {
    fd = open(dev, O_RDWR | O_NOCTTY | O_SYNC);
    if (fd < 0) {
        perror("open");
        exit(1);
    }

    struct termios tty;
    cfsetospeed (&tty, BAUDRATE);
    cfsetispeed (&tty, BAUDRATE);
    tty.c_cflag = BAUDRATE | CS8 | CLOCAL | CREAD | CRTSCTS;
    tty.c_iflag = IGNPAR;
    tty.c_oflag = 0;
    /* set input mode (non-canonical, no echo,...) */
    tty.c_lflag = ICANON;

    /* no read timeout */
    tty.c_cc[VTIME] = 0;

    /* read(2) blocks until MIN bytes are available,
     * and returns up to the number of bytes requested. */
    tty.c_cc[VMIN] = 64;
    tcsetattr(fd, TCSANOW, &tty);

    send_command(CLEAR, 0, 0, 0, 0);
    send_command(SET_BRIGHTNESS, 240, 0, 0, 0);
}

void lcd::init_font(char *path) {
    struct stat st;
    if (stat("courier.ttf", &st) < 0) {
        perror("stat");
        exit(1);
    }
    FILE *fp = fopen(path, "rb");
    if (fp == NULL) {
        perror("fopen");
        exit(1);
    }
    font_buf = (uint8_t*) malloc(st.st_size);
    fread(font_buf, st.st_size, 1, fp);
    fclose(fp);

    stbtt_InitFont(&font, font_buf, stbtt_GetFontOffsetForIndex(font_buf, 0));

    scale = stbtt_ScaleForPixelHeight(&font, 20);

    /* Add padding for out of bounds glyphs */
    int ascent;
    stbtt_GetFontVMetrics(&font, &ascent, &descent, 0);
    char_height = (ascent - descent) * scale;
    descent *= scale;

    /* All character widths are equal in monospace fonts */
    stbtt_GetCodepointHMetrics(&font, ' ', &char_width, 0);
    char_width *= scale;

    xoff = 2;
}

void lcd::write_line(std::string text, int line) {
    if ((int) text.size() > (WIDTH - xoff) / char_width) {
        fprintf(stderr, "text is too wide\n");
        exit(1);
    }
    if (line >= HEIGHT / char_height) {
        fprintf(stderr, "line is too high\n");
        exit(1);
    }
    int bottom = char_height * (line + 1);
    for (size_t i = 0; i < text.size(); i++) {
        rasterize_char(text[i], bottom, i);
    }

    send_command(DISPLAY_BITMAP, 0, bottom - char_height, WIDTH - 1, bottom - 1);

    uint16_t buf[WIDTH * char_height];
    for (int j = 0; j < char_height; j++) {
        for (int i = 0; i < WIDTH; i++) {
            uint8_t val = screen[(bottom - char_height) + j][i];
            buf[i + (j * WIDTH)] = pack_rgb(val, 0, 0, 0);
            screen[(bottom - char_height) + j][i] = 1;
        }
    }

    write(fd, buf, WIDTH * char_height * 2);
    tcdrain(fd);
}

void lcd::write_char(char c, int line, int col) {
    int bottom = char_height * (line + 1);
    rasterize_char(c, bottom, col);

    int x = (col * char_width) + xoff;
    send_command(DISPLAY_BITMAP, x, bottom - char_height, x + char_width - 1, bottom - 1);

    uint16_t buf[char_width * char_height];
    for (int j = 0; j < char_height; j++) {
        for (int i = 0; i < char_width; i++) {
            uint8_t val = screen[(bottom - char_height) + j][i + x];
            buf[i + (j * char_width)] = pack_rgb(val, 0, 0, 0);
            screen[(bottom - char_height) + j][i + x] = 0;
        }
    }

    write(fd, buf, char_width * char_height * 2);
    tcdrain(fd);
}
