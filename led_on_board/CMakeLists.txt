cmake_minimum_required(VERSION 3.13)
include($ENV{PICO_SDK_PATH}/external/pico_sdk_import.cmake)

project(led_on_board)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

pico_sdk_init()

add_executable(led_on_board led_on_board.c)

target_link_libraries(led_on_board pico_stdlib pico_cyw43_arch_none)

pico_add_extra_outputs(led_on_board)
