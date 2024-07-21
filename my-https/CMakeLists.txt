cmake_minimum_required(VERSION 3.13)

include($ENV{PICO_SDK_PATH}/external/pico_sdk_import.cmake)

project(my-https_project C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
pico_sdk_init()

add_executable(my-https
  tls.c
  tls_common.c
  main.c
)

pico_enable_stdio_usb(my-https 1)
pico_enable_stdio_uart(my-https 0)

pico_add_extra_outputs(my-https)

target_include_directories(my-https PRIVATE
  ${CMAKE_CURRENT_LIST_DIR}
)

target_link_libraries(my-https
  pico_cyw43_arch_lwip_poll
  pico_stdlib
  pico_mbedtls
  pico_lwip_mbedtls
)

# Ignore warnings from lwip code
set_source_files_properties(
  $ENV{PICO_SDK_PATH}/lib/lwip/src/apps/altcp_tls/altcp_tls_mbedtls.c
  PROPERTIES
  COMPILE_OPTIONS "-Wno-unused-result"
)

