#include "pico/stdlib.h"
#include <stdio.h>

int main() {
  uint cnt = 0;
  stdio_init_all();
  while (true) {
    printf("Hello, world!%d\n", cnt++);
    sleep_ms(5000);
  }
}
