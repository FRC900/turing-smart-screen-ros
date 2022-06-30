cmake_minimum_required(VERSION 2.8.3)
project(turing_smart_screen)

include("../cmake_modules/CMakeOpt.cmake")

find_package(catkin REQUIRED COMPONENTS roscpp std_msgs genmsg)

add_message_files(FILES line.msg)

generate_messages(DEPENDENCIES std_msgs)

catkin_package()

include_directories(include ${catkin_INCLUDE_DIRS})

add_executable(turing_smart_screen src/font.cpp src/lcd.cpp src/ros.cpp)
target_link_libraries(turing_smart_screen ${catkin_LIBRARIES})
