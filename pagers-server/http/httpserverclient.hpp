#pragma once

#include <lwip/tcp.h>
#include <circularbuffer.hpp>
#include <map>
#include <string>

#include "ArduinoJson.h"

enum class Method {
    GET,
    POST,
    INVALID,
};

class HttpServer;

class HttpServerClient {

    using MapStrings = std::map<std::string, std::string>;

    tcp_pcb* const pcb;
    CircularBuffer buf;

    bool first_line = true;
    Method method;
    char req_path[1024];
    MapStrings request_headers;
    MapStrings request_params;

    MapStrings response_headers;
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

    // returns char read
    int parse_url_arguments(const char* ptr);
    int parse_header(const char* ptr);
    void recv_data(struct pbuf *p);
    int recv_line();
    void handle_body();

    err_t flush();
    err_t close();
    void send_ack(int b) { sent += b; }
    bool send_finished() { return sent == should_send; }

    void send_string(const char* str);
    void response_begin(int code, const char* code_str);
    void not_found();

    // to be called from callbacks
    MapStrings get_req_headers() { return request_headers; }
    bool has_req_header(const std::string& hdr) { return request_headers.count(hdr) > 0; }
    std::string get_req_header(const std::string& hdr) { return request_headers.at(hdr); }
    int get_req_header_int(const std::string& hdr) { return stoi(get_req_header(hdr)); }

    MapStrings get_req_params() { return request_params; }
    bool has_req_param(const std::string& p) { return request_params.count(p) > 0; }
    std::string get_req_param(const std::string& p) { return request_params.at(p); }
    int get_req_param_int(const std::string& p) { return stoi(get_req_param(p)); }

    void set_header(const char* name, const char* val) { response_headers[name] = val; }
    void set_content_length(int len) { response_headers["Content-Length"] = std::to_string(len); }
    void set_content_type(const char* type) { response_headers["Content-Type"] = type; }
    void set_json() { set_content_type("application/json"); }

    void response_ok(const char* str);
    void response_err(const char* str);
    void response_bad(const char* str);
    void response_json(DynamicJsonDocument& json_doc, char* buf, int bufsize);
};

using ClientPtr = HttpServerClient*;