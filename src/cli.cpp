#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include "font.h"
#include "lcd.h"

#define ASSERT(a) if (!(a)) { fputs("Invalid arguments\n", stderr); usage(); exit(1); }
#define DEFAULT_OUT (char *) "/dev/ttyACM0"

void usage() {
    fputs("tss write -t TEXT -i INPUT -s SIZE [-c R,G,B] [-o OUTPUT] [-p LINE,COL]\n"
          "    Writes a line of text to the screen, throwing an error if the text wouldn't fit\n"
          "    TEXT: text to write to the turing smart screen\n"
          "    INPUT: path to a .ttf file to render glyphs from, recommended monospace font\n"
          "    SIZE: height, in pixels, of the glyphs to draw, recommended 20\n"
          "    R,G,B: float values between 0 and 1 of color to draw text in, default (0, 0, 0)\n"
          "    OUTPUT: path to serial port device to write to, default \"/dev/ttyACM0\"\n"
          "    LINE,COL: Zero indexed point on screen to start writing, default (0,0)\n\n"
          "tss brightness -b BRIGHTNESS [-o OUTPUT]\n"
          "    Sets the screen brightness. Recommended to decrease so the screen doesn't overheat\n"
          "    BRIGHTNESS: float value of brightness between 0 and 1 to set the screen to\n"
          "    OUTPUT: path to serial port device to write to, default \"/dev/ttyACM0\"\n\n"
          "tss clear [-o OUTPUT]\n"
          "    Quickly fills the screen with white pixels\n"
          "    OUTPUT: path to serial port device to write to, default \"/dev/ttyACM0\"\n"
          , stderr);
}

int main(int argc, char *argv[]) {
    /* Start `getopt()` on the third argument */
    optind++;

    /* Print help and exit if run without options or with one of "-h", "help", or "--help". */
    if (argc < 2 || !strcmp(argv[1], "-h") || !strcmp(argv[1], "help") || !strcmp(argv[1], "--help")) {
        usage();
    } else if (!strcmp(argv[1], "brightness")) {
        char *out = DEFAULT_OUT;
        float brightness = -1;
        char c;
        while ((c = getopt(argc, argv, "b:o:")) != -1) {
            int ret = 1;
            ASSERT(optarg);
            switch (c) {
                case 'b':
                    ret = sscanf(optarg, "%f", &brightness);
                    break;
                case 'o':
                    out = optarg;
                    break;
            }
            ASSERT(ret == 1);
        }

        ASSERT(brightness != -1);

        lcd lcd(out);

        /* For some reason higher values make the screen darker */
        lcd.set_brightness((1 - brightness) * 255);
    } else if (!strcmp(argv[1], "clear")) {
        char *out = DEFAULT_OUT;
        char c;
        while ((c = getopt(argc, argv, "o:")) != -1) {
            ASSERT(optarg);
            switch (c) {
                case 'o':
                    out = optarg;
                    break;
            }
        }

        lcd lcd(out);
        lcd.clear();
    } else if (!strcmp(argv[1], "write")) {
        char *text = NULL;
        char *out = DEFAULT_OUT;
        char *in = NULL;
        int size = 0;
        int line = 0, col = 0;
        float r = 0, g = 0, b = 0;

        char c;
        while ((c = getopt(argc, argv, "t:i:s:c:o:p:")) != -1) {
            int ret = 1;
            ASSERT(optarg);
            switch (c) {
                case 't':
                    text = optarg;
                    break;
                case 'i':
                    in = optarg;
                    break;
                case 's':
                    ret = sscanf(optarg, "%d", &size);
                    break;
                case 'c':
                    ret = sscanf(optarg, "%f,%f,%f", &r, &g, &b) - 2;
                    break;
                case 'o':
                    out = optarg;
                    break;
                case 'p':
                    ret = sscanf(optarg, "%d,%d", &line, &col) - 1;
                    break;
                default:
                    printf("%c\n", c);
            }
            ASSERT(ret == 1);
        }

        ASSERT(text && in && size);

        font font(in, size);
        lcd lcd(out);
        rgb color = {r, g, b};
        lcd.write_text(font, text, line, col, color);
    }
}
