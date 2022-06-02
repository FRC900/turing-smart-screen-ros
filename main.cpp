#include "font.h"
#include "lcd.h"
#include <unistd.h>
#include <signal.h>
#include <sys/signalfd.h>
#include <sys/epoll.h>

volatile sig_atomic_t sig = 0;

void sig_handler(int i) {
    sig = 1;
}

void check_sig() {
    if (sig) {
        exit(0);
    }
}

int main(int argc, char *argv[]) {
    srand(time(NULL));
    font font((char *)"courier.ttf", 20);
    lcd lcd((char *)"/dev/ttyACM0");
    rgb color = {0, 0, 0};

    signal(SIGINT, sig_handler);
    lcd.write_text(font, "Write a line at a time.", 0, 0, color);
    sleep(2);

    check_sig();
    lcd.write_text(font, "Write a character at a time.", 1, 0, color);
    sleep(2);

    for (int i = 0; i < 26; i++) {
        check_sig();
        char c = 'a' + i;
        lcd.write_text(font, &c, 2, i, color);
        usleep(200000);
    }
    sleep(1);


    check_sig();
    lcd.write_text(font, "Write a character anywhere", 3, 0, color);
    sleep(2);

    for (int i = 0; i < 1000; i++) {
        check_sig();
        char c = 'a' + rand() % 26;
        lcd.write_text(font, &c, rand() % 24, rand() % 28, color);
    }
}
