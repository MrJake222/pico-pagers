#pragma once

#include <lwip/tcp.h>
#include <circularbuffer.hpp>
#include <map>
#include <string>

#include "ArduinoJson.h"

enum class Method {
    GET,
    INVALID
};

class HttpServer;

class HttpServerClient {

    tcp_pcb* const pcb;
    CircularBuffer buf;

    bool first_line = true;
    Method method;
    char req_path[1024];

    std::map<std::string, std::string> headers;
    int sent = 0;
    int should_send = 0;

public:
    HttpServer* const server;

    HttpServerClient(HttpServer* server_, tcp_pcb* pcb_)
        : server(server_)
        , pcb(pcb_)
        , buf(1024, 128)
        { }

    // to be called from server
    Method get_method() { return method; }
    const char* get_path() { return req_path; }

    void recv_data(struct pbuf *p);
    int recv_line();

    err_t flush();
    err_t close();
    void send_ack(int b) { sent += b; }
    bool send_finished() { return sent == should_send; }

    void send_string(const char* str);
    void response_begin(int code, const char* code_str);
    void not_found();

    // to be called from callbacks
    void set_header(const char* name, const char* val) { headers[name] = val; }
    void set_content_length(int len) { headers["Content-length"] = std::to_string(len); }
    void set_content_type(const char* type) { headers["Content-type"] = type; }
    void set_json() { set_content_type("application/json"); }

    void response_ok(const char* str);
    void response_json(DynamicJsonDocument& json_doc, char* buf, int bufsize);
};

using ClientPtr = HttpServerClient*;