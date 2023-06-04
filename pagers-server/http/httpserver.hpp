#pragma once

#include <lwip/tcp.h>

class HttpServer {

    using CbFunc = void(*)(void* arg);

    struct tcp_pcb *server_pcb;

public:
    bool start(int port);

    void on(const char* path, CbFunc callback);
    void static_content(const char* path);
};

using ServerPtr = HttpServer*;