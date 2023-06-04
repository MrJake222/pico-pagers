#include "httpserver.hpp"
#include "httpserverclient.hpp"

#include <cstdio>
#include <pico/cyw43_arch.h>

static err_t tcp_close_client_connection(ClientPtr client, err_t close_err) {
    if (client) {
        close_err = client->close();
        delete client;
    }

    return close_err;
}

static void tcp_handle_request(ClientPtr client) {
    printf("handling request\n");
    printf("method: %d\n", client->get_method());
    printf("path: %s\n\n", client->get_path());
}

err_t tcp_client_recv(void *arg, struct tcp_pcb *pcb, struct pbuf *p, err_t err) {
    auto client = (ClientPtr)arg;

    if (p == nullptr) {
        // client closed the connection
        tcp_close_client_connection(client, err);
        return ERR_ABRT;
    }

    client->recv_data(p);

    printf("received %d bytes, err=%d\n", p->len, err);

    // scan newly received data
    for (int i=0; i<p->len; i++) {
        if (((uint8_t*)p->payload)[i] == '\n') {
            // received whole line
            int len = client->recv_line();
            if (len < 0) {
                // error
            }
            else if (len == 0) {
                // finished
                tcp_handle_request(client);
            }
            else {
                // processed the line
                tcp_recved(pcb, len);
            }
        }
    }

    pbuf_free(p);
    return ERR_OK;
}

static void tcp_client_err(void *arg, err_t err) {
    auto client = (ClientPtr)arg;
    if (err != ERR_ABRT) {
        printf("tcp_client_err_fn %d\n", err);
        tcp_close_client_connection(client, err);
    }
}

static err_t tcp_server_accept(void *arg, struct tcp_pcb *client_pcb, err_t err) {
    auto http = (ServerPtr)arg;
    if (err != ERR_OK || client_pcb == NULL) {
        printf("failure in accept, err=%d\n", err);
        return ERR_VAL;
    }
    printf("client connected\n");

    // Create the state for the connection
    auto* client = new HttpServerClient(client_pcb);
    if (!client) {
        printf("failed to allocate client state\n");
        return ERR_MEM;
    }

    // setup connection to client
    tcp_arg(client_pcb, client);
    // tcp_sent(client_pcb, tcp_server_sent);
    tcp_recv(client_pcb, tcp_client_recv);
    // tcp_poll(client_pcb, tcp_server_poll, POLL_TIME_S * 2);
    tcp_err(client_pcb, tcp_client_err);

    return ERR_OK;
}

bool HttpServer::start(int port) {

    cyw43_arch_lwip_begin();

    struct tcp_pcb *pcb = tcp_new();
    if (!pcb) {
        printf("failed to create pcb\n");
        cyw43_arch_lwip_end();
        return false;
    }

    err_t err = tcp_bind(pcb, IP_ANY_TYPE, port);
    if (err) {
        printf("failed to bind to port %d\n", port);
        cyw43_arch_lwip_end();
        return false;
    }

    server_pcb = tcp_listen_with_backlog(pcb, 1);
    if (!server_pcb) {
        printf("failed to listen\n");
        tcp_close(pcb);
        cyw43_arch_lwip_end();
        return false;
    }

    tcp_arg(server_pcb, this);
    tcp_accept(server_pcb, tcp_server_accept);

    cyw43_arch_lwip_end();
    return true;
}
