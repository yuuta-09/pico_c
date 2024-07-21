#include <string.h>

#include "lwip/dns.h"
#include "lwip/inet.h"
#include "lwip/pbuf.h"
#include "lwip/tcp.h"
#include "pico/cyw43_arch.h"
#include "pico/stdlib.h"
#include "secret.h"

#define HOSTNAME "example.com" // host名
#define PATH "/index.html"     // path
#define HTTP_PORT 80           // http通信をする際のポート番号

/*
 * データの送信が行われた場合に呼び出されるコールバック関数
 */
static err_t http_sent(void *arg, struct tcp_pcb *pcb, u16_t len) {
  printf("Data sent\n");
  return ERR_OK;
}

/*
 * 接続でデータを取得した際に呼び出されるコールバック関数
 */
static err_t http_recv(void *arg, struct tcp_pcb *pcb, struct pbuf *p,
                       err_t err) {
  // 受信したデータがNULLだった場合接続を閉じる
  if (p == NULL) {
    tcp_close(pcb);
    printf("Connection closed\n");
    return ERR_OK;
  }

  // 受信したデータを表示したのち削除
  printf("Received data: %.*s\n", p->len, (char *)p->payload);
  pbuf_free(p);
  return ERR_OK;
}

static void dns_callback(const char *name, const ip_addr_t *ipaddr,
                         void *callback_arg) {
  // ipaddrが指定されていない場合はエラー
  if (!ipaddr) {
    printf("DNS resolution failed\n");
    return;
  }

  // 新しくTCPプロトコルを作成
  struct tcp_pcb *pcb = tcp_new();
  if (!pcb) {
    printf("Error creating PCB\n");
    return;
  }

  // pcbを使用して別のホストに接続
  err_t err = tcp_connect(pcb, ipaddr, HTTP_PORT, NULL);
  if (err != ERR_OK) {
    printf("Error connecting to server: %d\n", err);
    tcp_close(pcb);
    return;
  }

  char request[256]; // 送信するデータを格納する
  snprintf(request, sizeof(request), "GET %s HTTP/1.1\r\nHost: %s\r\n\r\n",
           (char *)callback_arg, name);

  // 送受信成功時のコールバック関数を設定
  tcp_sent(pcb, http_sent);
  tcp_recv(pcb, http_recv);

  // 送信用のデータを書き込み
  tcp_write(pcb, request, strlen(request), TCP_WRITE_FLAG_COPY);
}

static void http_request(const char *hostname, const char *path) {
  ip_addr_t ipaddr; // ip addressを格納する変数

  // ホスト名をip addressに変換
  err_t err = dns_gethostbyname(hostname, &ipaddr, dns_callback, (void *)path);
  if (err != ERR_INPROGRESS) {
    printf("DNS resolution failed: %d\n", err);
  }
}

void init_wifi() {
  if (cyw43_arch_init()) {
    printf("failed to initialize\n");
    exit(EXIT_FAILURE);
  }

  // 他のWi-Fiアクセスポイントに接続を可能に
  cyw43_arch_enable_sta_mode();

  // ワイヤレスアクセスポイントへの接続試行
  while (cyw43_arch_wifi_connect_blocking(WIFI_SSID, WIFI_PASS,
                                          CYW43_AUTH_WPA2_AES_PSK)) {
    printf("failed to connect\n");
  }

  printf("Connected to Wi-Fi\n");

  return;
}

int main() {
  stdio_init_all(); // 標準入出力の初期化
  dns_init();

  init_wifi();

  // http通信開始
  http_request(HOSTNAME, PATH);

  // pico_cyw43_archを使用する場合は定期的に呼び出す必要あり
  while (true) {
    cyw43_arch_poll();
    sleep_ms(1000);
  }

  // ワイヤレス通信終了
  cyw43_arch_deinit();

  return EXIT_SUCCESS;
}
