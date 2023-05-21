#include <stdio.h>
#include <pico/stdlib.h>
#include <pico/cyw43_arch.h>
#include <hardware/uart.h>

#include "lwip/pbuf.h"
#include "lwip/tcp.h"

#include "physical.hpp"
#include "dhcpserver/dhcpserver.h"
#include "dnsserver.h"
#include "dhcpserver/dhcpserver.h"

// hardware UART on physical pins
#define UART_ID uart0
#define BAUD_RATE 9600
#define UART_TX_PIN 0
#define UART_RX_PIN 1

#include <protocol.hpp>

#define TCP_PORT 80
char ap_name[] = "pagers-server";
char ap_password[] = "password";

typedef struct TCP_SERVER_T_ {
    struct tcp_pcb *server_pcb;
    bool complete;
    ip_addr_t gw;
} TCP_SERVER_T;

typedef struct TCP_CONNECT_STATE_T_ {
    struct tcp_pcb *pcb;
    int sent_len;
    char headers[128];
    char result[256];
    int header_len;
    int result_len;
    ip_addr_t *gw;
} TCP_CONNECT_STATE_T;

static err_t tcp_close_client_connection(TCP_CONNECT_STATE_T *con_state, struct tcp_pcb *client_pcb, err_t close_err) {
    if (client_pcb) {
        assert(con_state && con_state->pcb == client_pcb);
        tcp_arg(client_pcb, NULL);
        tcp_poll(client_pcb, NULL, 0);
        tcp_sent(client_pcb, NULL);
        tcp_recv(client_pcb, NULL);
        tcp_err(client_pcb, NULL);
        err_t err = tcp_close(client_pcb);
        if (err != ERR_OK) {
            printf("close failed %d, calling abort\n", err);
            tcp_abort(client_pcb);
            close_err = ERR_ABRT;
        }
        if (con_state) {
            free(con_state);
        }
    }
    return close_err;
}

static err_t tcp_server_sent(void *arg, struct tcp_pcb *pcb, u16_t len) {
    TCP_CONNECT_STATE_T *con_state = (TCP_CONNECT_STATE_T*)arg;
    printf("tcp_server_sent %u\n", len);
    con_state->sent_len += len;
    if (con_state->sent_len >= con_state->header_len + con_state->result_len) {
        printf("all done\n");
        return tcp_close_client_connection(con_state, pcb, ERR_OK);
    }
    return ERR_OK;
}

static void tcp_server_err(void *arg, err_t err) {
    TCP_CONNECT_STATE_T *con_state = (TCP_CONNECT_STATE_T*)arg;
    if (err != ERR_ABRT) {
        printf("tcp_client_err_fn %d\n", err);
        tcp_close_client_connection(con_state, con_state->pcb, err);
    }
}

static err_t tcp_server_accept(void *arg, struct tcp_pcb *client_pcb, err_t err) {
    TCP_SERVER_T *state = (TCP_SERVER_T*)arg;
    if (err != ERR_OK || client_pcb == NULL) {
        printf("failure in accept\n");
        return ERR_VAL;
    }
    printf("client connected\n");

    // Create the state for the connection
    TCP_CONNECT_STATE_T *con_state = static_cast<TCP_CONNECT_STATE_T *>(calloc(1, sizeof(TCP_CONNECT_STATE_T)));
    if (!con_state) {
        printf("failed to allocate connect state\n");
        return ERR_MEM;
    }
    con_state->pcb = client_pcb; // for checking
    con_state->gw = &state->gw;

    // setup connection to client
    tcp_arg(client_pcb, con_state);
//    tcp_sent(client_pcb, tcp_server_sent);
//    tcp_recv(client_pcb, tcp_server_recv);
//    tcp_poll(client_pcb, tcp_server_poll, POLL_TIME_S * 2);
    tcp_err(client_pcb, tcp_server_err);

    return ERR_OK;
}

static bool tcp_server_open(void *arg) {
    TCP_SERVER_T *state = (TCP_SERVER_T*)arg;
    printf("starting server on port %u\n", TCP_PORT);

    struct tcp_pcb *pcb = tcp_new_ip_type(IPADDR_TYPE_ANY);
    if (!pcb) {
        printf("failed to create pcb\n");
        return false;
    }

    err_t err = tcp_bind(pcb, IP_ANY_TYPE, TCP_PORT);
    if (err) {
        printf("failed to bind to port %d\n");
        return false;
    }

    state->server_pcb = tcp_listen_with_backlog(pcb, 1);
    if (!state->server_pcb) {
        printf("failed to listen\n");
        if (pcb) {
            tcp_close(pcb);
        }
        return false;
    }

    tcp_arg(state->server_pcb, state);
    tcp_accept(state->server_pcb, tcp_server_accept);

    return true;
}


int main() {

    // UART on USB
    stdio_usb_init();

    sleep_ms(2000);
    printf("\n\nHello usb pagers-server!\n");
    config_print();
    for (int i = 5; i > 0; i--) {
        printf("Booting in %d seconds...\n", i);
        sleep_ms(1000);
    }

    TCP_SERVER_T *state = static_cast<TCP_SERVER_T *>(calloc(1, sizeof(TCP_SERVER_T)));
    if (!state) {
        printf("failed to allocate state\n");
        return 1;
    }

    printf("cyw43 initialization...\n");
    if (cyw43_arch_init_with_country(CYW43_COUNTRY_POLAND)) {
        printf("cy43 init failed\n");
        return 1;
    };
    printf("cyw43 initialised\n");

    cyw43_arch_enable_ap_mode(ap_name, ap_password, CYW43_AUTH_WPA2_AES_PSK);

    ip4_addr_t mask;
    IP4_ADDR(ip_2_ip4(&state->gw), 192, 168, 4, 1);
    IP4_ADDR(ip_2_ip4(&mask), 255, 255, 255, 0);

    // Start the dhcp server
    dhcp_server_t dhcp_server;
    dhcp_server_init(&dhcp_server, &state->gw, &mask);

    // Start the dns server
    dns_server_t dns_server;
    dns_server_init(&dns_server, &state->gw);

    if (!tcp_server_open(state)) {
        printf("failed to open server\n");
        return 1;
    }

    /**
     *
     * REST OF THE CODE
     *
     */

    // hardware UART
    /*uart_init(UART_ID, BAUD_RATE);
    gpio_set_function(UART_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(UART_RX_PIN, GPIO_FUNC_UART);*/

    // printf writes to USB only
    // uart_* write to pin GP(UART_TX_PIN) only

    struct proto_data data = {
            .sequence_number = 0, //0xCAFEDEADBEEFCAFE,
            .receiver_id = 0x1215,
            .message_type = 0xDBDB,
            .message_param = 0xACDC
    };

    const uint8_t private_key[KEY_LENGTH_BYTES] = { 0 };
    struct proto_frame frame;

    send_setup();

    // for testing
    gpio_init(2);
    gpio_set_dir(2, GPIO_OUT);

    proto_checksum_calc(&data);
    proto_encrypt(private_key, &data, &frame);
    printf("sending: ");
    for (int i=0; i<PROTO_DATA_SIZE; i++)
        printf("%02x ", ((uint8_t*)&frame)[i]);
    printf("\n");

    printf("sizeof proto_data %u\n", sizeof(struct proto_data));
    printf("sizeof proto_frame %u\n", sizeof(struct proto_frame));

    while (1) {
        int cnt = CLOCK_SPEED_HZ * 1; // delay in seconds
        while (cnt--) send_silence();
        printf("running\n");


        data.sequence_number++;

        proto_checksum_calc(&data);
        proto_encrypt(private_key, &data, &frame);

        send_bytes((uint8_t*)&frame, PROTO_FRAME_SIZE);
    }
}