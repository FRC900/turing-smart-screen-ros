#include "lcd.h"
#include <termios.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <string.h>
#include "font.h"

void lcd::send_command(uint8_t cmd, int x, int y, int ex, int ey) {
    /* Pack 4 10-bit values and an 8-bit cmd into a 5-byte buffer */
    uint8_t buf[6] = {0};
    buf[0] = x >> 2;
    buf[1] = ((x &  3) << 6) | (y >> 4);
    buf[2] = ((y & 15) << 4) | (ex >> 6);
    buf[3] = ((ex & 63) << 2) | (ey >> 8);
    buf[4] = ey & 255;
    buf[5] = cmd;
    write(fd, buf, 6);
}

uint16_t lcd::pack_rgb(uint8_t signal, rgb c) {
    /* Linear interpolation: f(0) = (1,1,1), f(1) = (r, g, b) */
    float x = (float) signal / 0xFF;
    c.r = (1 - x) + (x * c.r);
    c.g = (1 - x) + (x * c.g);
    c.b = (1 - x) + (x * c.b);
    uint8_t r1 = (int)(c.r * 0xFF) >> 3;
    uint8_t g1 = (int)(c.g * 0xFF) >> 2;
    uint8_t b1 = (int)(c.b * 0xFF) >> 3;
    return (r1 << 11) | (g1 << 5) | b1;
}

lcd::lcd(char *dev) {
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

void lcd::write_text(font &font, std::string text, int line, int col, rgb color) {
    int xpos = col * font.char_width;
    int ypos = line * font.char_height;

    if ((line + 1) * font.char_height > HEIGHT) {
        fputs("text is beyond bottom of bounding box\n", stderr);
        exit(1);
    }
    if ((text.size() + col) * font.char_width > WIDTH) {
        fputs("text is beyond side of bounding box\n", stderr);
        exit(1);
    }

    int width = text.size() * font.char_width;

    /* All pixels start white */
    std::fill_n(screen, width * font.char_height, 0xFFFF);

    for (size_t i = 0; i < text.size(); i++) {
        glyph &g = font[text[i]];
        int glyph_offset = i * font.char_width;
        for (int y = 0; y < g.dim.y; y++) {
            for (int x = 0; x < g.dim.x; x++) {
                /* Glyph offset + line offset + pixel offset */
                int j = glyph_offset + (y + g.box_off.y) * width + x + g.box_off.x;
                screen[j] = pack_rgb(g(x, y), color);
            }
        }
    }


    send_command(DISPLAY_BITMAP, xpos, ypos, xpos + width - 1, ypos + font.char_height - 1);
    write(fd, screen, width * font.char_height * 2);
}

lcd::~lcd() {
    close(fd);
}
