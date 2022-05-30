#include "lcd.h"
#include <unistd.h>

int main(int argc, char *argv[]) {
    srand(time(NULL));
    lcd lcd;
    lcd.init_lcd((char *)"/dev/ttyACM0");
    lcd.init_font((char *)"courier.ttf");
    lcd.write_line("Write a line at a time.", 0);
    sleep(2);
    lcd.write_line("Write a character at a time.", 0);
    sleep(1);
    for (int i = 0; i < 26; i++) {
        lcd.write_char('a' + i, 1, i);
        usleep(250000);
    }
    sleep(1);
    lcd.write_line("Write a character anywhere", 0);
    while (true) {
        lcd.write_char('a' + rand() % 26, rand() % 24, rand() % 28);
    }
}
