#include "font.h"
#include "lcd.h"
#include <unistd.h>
#include <signal.h>
#include <sys/signalfd.h>
#include <sys/epoll.h>
#include <turing_smart_screen/line.h>
#include "ros/ros.h"

font font((char *)"courier.ttf", 20);
lcd display((char *)"/dev/ttyACM0");

struct line {
    rgb color;
    std::string text;
};

std::vector<line> lines;

void update_line(std::string &text, size_t line, rgb color) {
    if (line >= lines.size()) {
        lines.resize(line + 1);
    }
    int trailing = lines[line].text.size() - text.size();
    if (trailing > 0) {
        std::string space(trailing, ' ');
        display.write_text(font, space, line, text.size(), color);
    }

    if (lines[line].color != color){
        display.write_text(font, text, line, 0, color);
    } else {
        size_t j = 0;
        for (size_t i = 0; i < lines[line].text.size() && i < text.size(); i++) {
            if (lines[line].text[i] == text[i]) {
                if (i != j) {
                    std::string_view v = std::string_view(text).substr(j, i - j);
                    display.write_text(font, v, line, j, color);
                }
                j = i + 1;
            }
        }
        std::string_view v = std::string_view(text).substr(j, text.size() - j);
        display.write_text(font, v, line, 0, color);
    }
    lines[line].color = color;
    lines[line].text = text;
}

void callback(turing_smart_screen::line line) {
    update_line(line.text, line.line, {line.r, line.g, line.b});
}

int main(int argc, char *argv[]) {
    ROS_INFO("starting turing_smart_screen node");
    display.clear();

    ros::init(argc, argv, "lcd");
    ros::NodeHandle nh("~");
    ros::Subscriber sub = nh.subscribe("/status", 1, callback);

    ros::spin();
}
