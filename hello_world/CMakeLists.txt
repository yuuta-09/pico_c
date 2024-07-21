cmake_minimum_required(VERSION 3.13)
include($ENV{PICO_SDK_PATH}/external/pico_sdk_import.cmake)

project(hello)

pico_sdk_init()

add_executable(hello_usb
  hello_usb.c
)

target_link_libraries(hello_usb pico_stdlib)

pico_enable_stdio_usb(hello_usb 1)
pico_enable_stdio_uart(hello_usb 0)

pico_add_extra_outputs(hello_usb)

