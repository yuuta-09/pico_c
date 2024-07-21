cmake_minimum_required(VERSION 3.13)
include($ENV{PICO_SDK_PATH}/external/pico_sdk_import.cmake)

project(pico_led_blink C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

pico_sdk_init()

add_executable(pico_led_blink
  main.c
)

target_link_libraries(pico_led_blink pico_stdlib)
pico_add_extra_outputs(pico_led_blink)
