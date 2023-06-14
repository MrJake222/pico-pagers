#pragma once

#include <lwip/tcp.h>
#include <map>
#include <string>
#include <lfs.h>
#include <vector>
#include "httpserverclient.hpp"

class HttpServer {
public:
    using CbKey = std::pair<Method, std::string>;
    using CbFunc = void(*)(HttpServerClient* client, void* arg);
    using ClientVect = std::vector<HttpServerClient*>;

private:
    void* cb_arg;
    std::map<CbKey, CbFunc> paths;

    struct tcp_pcb *server_pcb;

    bool static_enabled = false;
    lfs_t* lfs;
    const char* fs_path;
    bool try_static(HttpServerClient* client, const char* req_path);

    ClientVect client_vect;
    void handle_request(HttpServerClient* client, Method method, const char* path);

public:
    // used by callbacks
    void new_client(HttpServerClient* client) { client_vect.push_back(client); }

    // used by user
    bool start(int port);
    void set_cb_arg(void* arg) { cb_arg = arg; }
    void on(Method method, const char* path, CbFunc callback);
    // static content has lower priority than routes
    void static_content(lfs_t* lfs_, const char* fs_path_);

    // periodically called from main loop
    void loop();
};

using ServerPtr = HttpServer*;