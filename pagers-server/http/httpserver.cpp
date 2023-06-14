#include "httpserver.hpp"
#include "mime.hpp"

#include <cstdio>
#include <pico/cyw43_arch.h>

static err_t tcp_close_client_connection(struct tcp_pcb *pcb, ClientPtr client, err_t close_err) {
    if (pcb) {
        tcp_arg(pcb, nullptr);
    }

    if (client) {
        close_err = client->close();
        delete client;
    }

    return close_err;
}

static err_t tcp_server_sent(void *arg, struct tcp_pcb *pcb, u16_t len) {
    auto client = (ClientPtr)arg;

    if (client) {
        client->send_ack(len);
        if (client->send_finished()) {
            puts("closed sent all");
            tcp_close_client_connection(pcb, client, ERR_OK);
        }
    }

    return ERR_OK;
}

err_t tcp_client_recv(void *arg, struct tcp_pcb *pcb, struct pbuf *p, err_t err) {
    auto client = (ClientPtr)arg;

    if (!p || !client) {
        // client closed the connection
        puts("closed p null or client null");
        tcp_close_client_connection(pcb, client, err);
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
            else {
                // processed the line
                tcp_recved(pcb, len);

                if (len == 2) {
                    // only \r\n
                    // finished
                    client->mark_request_ready();
                }
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
        tcp_close_client_connection(nullptr, client, err);
    }
}

static err_t tcp_server_accept(void *arg, struct tcp_pcb *client_pcb, err_t err) {
    auto server = (ServerPtr)arg;
    if (err != ERR_OK || client_pcb == NULL) {
        printf("failure in accept, err=%d\n", err);
        return ERR_VAL;
    }
    printf("client connected\n");

    // Create the state for the connection
    auto* client = new HttpServerClient(server, client_pcb);
    if (!client) {
        printf("failed to allocate client state\n");
        return ERR_MEM;
    }

    // setup connection to client
    tcp_arg(client_pcb, client);
    tcp_sent(client_pcb, tcp_server_sent);
    tcp_recv(client_pcb, tcp_client_recv);
    // tcp_poll(client_pcb, tcp_server_poll, POLL_TIME_S * 2);
    tcp_err(client_pcb, tcp_client_err);

    // add to client list
    server->new_client(client);

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

    server_pcb = tcp_listen_with_backlog(pcb, 10);
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

bool HttpServer::try_static(HttpServerClient* client, const char* req_path) {
    if (!static_enabled)
        return false;

    const int BUFSZ = 128;
    char buf[BUFSZ];

    if (strcmp(req_path, "/") == 0)
        snprintf(buf, BUFSZ, "%s/index.html", fs_path);
    else
        snprintf(buf, BUFSZ, "%s%s", fs_path, req_path);
    lfs_file_t file;
    int res = lfs_file_open(lfs, &file, buf, LFS_O_RDONLY);
    if (res < 0) {
        return false;
    }

    client->set_content_type(content_type_for(buf));
    client->set_content_length(file.ctz.size);
    client->response_begin(200, "OK");

    int read;
    while (true) {
        read = lfs_file_read(lfs, &file, buf, BUFSZ-1);
        if (read < 0)
            return false;
        buf[read] = 0;
        client->send_string(buf);
        if (read < BUFSZ-1)
            break;
    }

    lfs_file_close(lfs, &file);

    return true;
}

void HttpServer::handle_request(HttpServerClient* client, Method method, const char* path) {
    client->handle_body();
    CbKey key = std::make_pair(method, path);

    if (paths.count(key) != 0) {
        paths.at(key)(client, cb_arg);
    } else if (method == Method::GET && try_static(client, path)) {
        // static
    } else {
        // no callback found & no static
        client->not_found();
    }

    // client->flush();
    // client->close();
}

void HttpServer::on(Method method, const char* path, HttpServer::CbFunc callback) {
    CbKey key = std::make_pair(method, path);
    paths[key] = callback;
}

void HttpServer::static_content(lfs_t* lfs_, const char* fs_path_) {
    static_enabled = true;
    lfs = lfs_;
    fs_path = fs_path_;
}

void HttpServer::loop() {
    for (ClientVect::iterator it = client_vect.begin(); it!=client_vect.end();) {
        auto client = *it;

        if (client->is_request_ready()) {
            // not efficient: this method will block if data can't be sent,
            // but after it returns, the client can be removed from the list
            handle_request(client, client->get_method(), client->get_path());

            it = client_vect.erase(it);
        }
        else {
            it++;
        }
    }
}
