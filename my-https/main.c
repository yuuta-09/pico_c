#include "pico/cyw43_arch.h"
#include "pico/stdlib.h"
#include "secret.h"
#include <stdio.h>
#include <time.h>

/*
 * Extern functions
 */
extern bool run_tls_client(const uint8_t *cert, size_t cert_len,
                           const char *server, const char *request,
                           int timeout);
/*
 * Define macros
 */
#define TLS_CLIENT_TIMEOUT_SEC 15
#define TLS_REQUEST_SIZE 4096

/*
 * Define functions
 */

// Connect to wifi
bool connect_to_wifit() {

  while (cyw43_arch_wifi_connect_blocking(SSID, PASSWORD,
                                          CYW43_AUTH_WPA2_AES_PSK)) {
    printf("failed to connect\n");
  }
  printf("Connected to Wi-Fi\n");
}

// Create request msg
void create_request_msg(char *request, const char *data) {
  snprintf(request, TLS_REQUEST_SIZE,
           "POST %s HTTP/1.1\r\n"
           "Host: %s\r\n"
           "Authorization: Bearer %s\r\n"
           "Content-Type: application/json\r\n"
           "Content-Length: %zu\r\n"
           "\r\n"
           "%s",
           PATH, SERVER, API_KEY, strlen(data), data);

  printf("**********************************\n");
  printf("request msg:\n%s\n", request);
  printf("**********************************\n");
}

int main() {
  // Define variables
  bool complete;
  clock_t start_clock, end_clock;
  const char *data =
      "{"
      "\"model\": \"gpt-4o-mini\","
      "\"messages\":["
      "{\"role\": \"user\", \"content\": \"Hello, how are you?\"}"
      "]"
      "}";
  char request[TLS_REQUEST_SIZE];

  // Initialize
  stdio_init_all();
  if (cyw43_arch_init()) {
    printf("failed to initialise\n");
    return false;
  }
  cyw43_arch_enable_sta_mode();

  // Start program
  printf("start program\n");
  printf("----------------------------------\n");

  start_clock = clock();
  connect_to_wifit();
  end_clock = clock();

  printf("Connect to wifi finished: %f\n",
         ((double)(end_clock - start_clock)) / CLOCKS_PER_SEC);

  create_request_msg(request, data);

  start_clock = clock();
  complete = run_tls_client(NULL, 0, SERVER, request, TLS_CLIENT_TIMEOUT_SEC);
  end_clock = clock();

  printf("TLS client finished: %f\n",
         ((double)(end_clock - start_clock)) / CLOCKS_PER_SEC);

  // End program and clean up
  printf("All done\n");
  printf("----------------------------------\n");

  cyw43_arch_deinit();

  return EXIT_SUCCESS;
}
