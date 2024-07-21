cmake_minimum_required(VERSION 3.13)

include($ENV{PICO_SDK_PATH}/external/pico_sdk_import.cmake)

project(http_project C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

pico_sdk_init()

add_executable(http
  http.c
)

target_include_directories(http PRIVATE
  ${CMAKE_CURRENT_LIST_DIR}
)

pico_enable_stdio_usb(http 1)
pico_enable_stdio_uart(http 0)

pico_add_extra_outputs(http)

target_link_libraries(http
  pico_cyw43_arch_lwip_threadsafe_background
  pico_stdlib
  pico_mbedtls
  pico_lwip_mbedtls
)
