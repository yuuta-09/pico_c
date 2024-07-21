#include "pico//stdlib.h"

int main() {
  const uint LED_PIN = 15; // Use GPIO15 pin

  // Set LED pin output mode
  gpio_init(LED_PIN);
  gpio_set_dir(LED_PIN, GPIO_OUT);

  while (1) {
    gpio_put(LED_PIN, 1);
    sleep_ms(1000);
    gpio_put(LED_PIN, 0);
    sleep_ms(1000);
  }
}
