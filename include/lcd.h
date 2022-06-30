#ifndef LCD_H_
#define LCD_H_

#include <cstdint>
#include <string>
#include <termios.h>
#include "font.h"

struct rgb {
    float r, g, b;
};

class lcd {
    const static int WIDTH = 320;
    const static int HEIGHT = 480;
    const static int PIXELS = WIDTH * HEIGHT;
    const static int BAUDRATE = B115200;
    const static int CLEAR = 102;
    const static int SET_BRIGHTNESS = 110;
    const static int DISPLAY_BITMAP = 197;

    int fd;
    uint16_t screen[PIXELS] = {};
    void send_command(uint8_t cmd, int x, int y, int ex, int ey);
    uint16_t pack_rgb(uint8_t signal, rgb c);
public:
    lcd(char *dev);
    void write_text(font &font, std::string text, int line, int col, rgb color);
    void clear();
    void set_brightness(int brightness);
    ~lcd();
};

#endif // LCD_H_
