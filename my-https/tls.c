#include <string.h>
#include <time.h>

#include "lwip/altcp.h"
#include "lwip/altcp_tls.h"
#include "lwip/dns.h"
#include "lwip/pbuf.h"
#include "pico/cyw43_arch.h"
#include "pico/stdlib.h"

/*
 * This file is a part of the PicoTCP project.
 * It is define the TLS functions
 *
 * You can check https://www.nongnu.org/lwip/2_1_x/index.html for more
 * information.
 */

/*
 * Define structs
 */
typedef struct TLS_CLIENT_T_ {
  struct altcp_pcb *pcb;
  bool complete;
  int error;
  const char *http_request;
  int timeout;
} TLS_CLIENT_T;

static struct altcp_tls_config *tls_config = NULL;

/*
 * Define functions
 */

static err_t tls_client_close(void *arg) {
  TLS_CLIENT_T *state = (TLS_CLIENT_T *)arg;
  err_t err = ERR_OK;

  state->complete = true;
  if (state->pcb != NULL) {
    altcp_arg(state->pcb, NULL);     // Set callback arg to NULL
    altcp_poll(state->pcb, NULL, 0); // Disable polling
    // Disable callback functions that will be called when data is received
    altcp_recv(state->pcb, NULL);
    // Disable callback functions that will be called when an error occurs
    altcp_err(state->pcb, NULL);
    err = altcp_close(state->pcb); // Close the connection

    if (err != ERR_OK) {
      printf("close failed %d, calling abort\n", err);
      altcp_abort(state->pcb); // Abort the connection
      err = ERR_ABRT;
    }
    state->pcb = NULL;
  }

  return err;
}

static err_t tls_client_connected(void *arg, struct altcp_pcb *pcb, err_t err) {
  TLS_CLIENT_T *state = (TLS_CLIENT_T *)arg;

  if (err != ERR_OK) {
    printf("connect failed %d\n", err);
    return tls_client_close(state);
  }

  printf("connected to server, sending request\n");
  // Send the HTTP request
  err = altcp_write(state->pcb, state->http_request,
                    strlen(state->http_request), TCP_WRITE_FLAG_COPY);

  if (err != ERR_OK) {
    printf("error writing data, err=%d", err);
    return tls_client_close(state);
  }

  return ERR_OK;
}

static err_t tls_client_poll(void *arg, struct altcp_pcb *pcb) {
  TLS_CLIENT_T *state = (TLS_CLIENT_T *)arg;
  state->error = PICO_ERROR_TIMEOUT;
  return tls_client_close(arg);
}

static void tls_client_err(void *arg, err_t err) {
  TLS_CLIENT_T *state = (TLS_CLIENT_T *)arg;
  printf("tls_client_err %d\n", err);
  tls_client_close(state);
  state->error = PICO_ERROR_GENERIC;
}

static err_t tls_client_recv(void *arg, struct altcp_pcb *pcb, struct pbuf *p,
                             err_t err) {
  TLS_CLIENT_T *state = (TLS_CLIENT_T *)arg;
  if (!p) {
    printf("connection closed\n");
    return tls_client_close(state);
  }

  if (p->tot_len > 0) {
    char buf[p->tot_len + 1];

    pbuf_copy_partial(p, buf, p->tot_len, 0);
    buf[p->tot_len] = 0;

    printf("***\nnew data received from server:\n***\n\n%s\n", buf);

    // The maximum amount of unackenowledged data that the receive can accept.
    altcp_recved(pcb, p->tot_len);
  }
  pbuf_free(p);

  return ERR_OK;
}

static void tls_client_connect_to_server_ip(const ip_addr_t *ipaddr,
                                            TLS_CLIENT_T *state) {
  err_t err;
  u16_t port = 443; // TLS port number

  printf("connecting to server IP %s port %d\n", ipaddr_ntoa(ipaddr), port);
  err = altcp_connect(state->pcb, ipaddr, port, tls_client_connected);

  if (err != ERR_OK) {
    fprintf(stderr, "error initiating connect, err=%d\n", err);
    tls_client_close(state);
  }
}

static void tls_client_dns_found(const char *hostname, const ip_addr_t *ipaddr,
                                 void *arg) {
  if (ipaddr) {
    printf("DNS resolving complete\n");
    tls_client_connect_to_server_ip(ipaddr, (TLS_CLIENT_T *)arg);
  } else {
    printf("error resolving hostname %s\n", hostname);
    tls_client_close(arg);
  }
}

static bool tls_client_open(const char *hostname, void *arg) {
  err_t err;
  ip_addr_t server_ip;
  TLS_CLIENT_T *state = (TLS_CLIENT_T *)arg;

  state->pcb = altcp_tls_new(tls_config, IPADDR_TYPE_ANY);
  if (!state->pcb) {
    printf("failed to create pcb\n");
    return false;
  }

  altcp_arg(state->pcb, state);
  altcp_poll(state->pcb, tls_client_poll, state->timeout * 2);
  altcp_recv(state->pcb, tls_client_recv);
  altcp_err(state->pcb, tls_client_err);

  /* Set SNI(Server Name Indication) */
  mbedtls_ssl_set_hostname(altcp_tls_context(state->pcb), hostname);
  printf("resolving %s\n", hostname);

  // Use for thread-safe
  cyw43_arch_lwip_begin();

  err = dns_gethostbyname(hostname, &server_ip, tls_client_dns_found, state);
  if (err == ERR_OK) {
    tls_client_connect_to_server_ip(&server_ip, state);
  } else if (err != ERR_INPROGRESS) {
    printf("error starting dns, err=%d\n", err);
    tls_client_close(state->pcb);
  }

  cyw43_arch_lwip_end();
  // end of thread-safe

  return err == ERR_OK || err == ERR_INPROGRESS;
}

static TLS_CLIENT_T *tls_client_init(void) {
  TLS_CLIENT_T *state = calloc(1, sizeof(TLS_CLIENT_T));
  if (!state) {
    printf("failed to allocate state\n");
    return NULL;
  }

  return state;
}

bool run_tls_client(const uint8_t *cert, size_t cert_len, const char *server,
                    const char *request, int timeout) {
  // create an ALTCP_TLS client configuuration handle
  tls_config = altcp_tls_create_config_client(cert, cert_len);

  TLS_CLIENT_T *state = tls_client_init();

  if (!state) {
    return false;
  }

  state->http_request = request;
  state->timeout = timeout;

  if (!tls_client_open(server, state)) {
    return false;
  }
  while (!state->complete) {
#if PICO_CYW43_ARCH_POLL
    cyw43_arch_poll();
    cyw43_arch_wait_for_work_until(make_timeout_time_ms(1000));
#else
    sleep_ms(1000);
#endif
  }

  int err = state->error;
  free(state);
  altcp_tls_free_config(tls_config);

  return err == 0;
}
