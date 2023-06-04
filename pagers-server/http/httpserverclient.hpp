#pragma once

#include <lwip/tcp.h>
#include <circularbuffer.hpp>

enum class Method {
    GET,
    INVALID
};

class HttpServerClient {

    tcp_pcb* pcb;
    CircularBuffer buf;

    Method method;
    char req_path[1024];

public:
    HttpServerClient(tcp_pcb* pcb_)
        : pcb(pcb_)
        , buf(1024, 128)
        { }

    void recv_data(struct pbuf *p);
    int recv_line();

    err_t close();

    Method get_method() { return method; }
    const char* get_path() { return req_path; }
};

using ClientPtr = HttpServerClient*;