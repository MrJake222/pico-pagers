#include <cstring>
#include "httpserverclient.hpp"

void HttpServerClient::recv_data(struct pbuf* p) {
    buf.write((uint8_t*)p->payload, p->len);
}

int HttpServerClient::recv_line() {
    // buf is set to the beginning of the line
    // less than \r\n
    if (buf.data_left_continuous() < 2) {
        buf.wrap_buffer();
    }

    int line_length = 0;
    if (first_line) {
        first_line = false;

        if (strncmp((char*) buf.read_ptr(), "GET", 3) == 0) {
            // GET method
            buf.read_ack(3 + 1); // move read ptr past GET+space
            line_length += 4;
            method = Method::GET;
        } else {
            method = Method::INVALID;
        }

        int req_path_len = 0;
        if (method != Method::INVALID) {
            // find space after request path
            char* endptr = strchr((char*) buf.read_ptr(), ' ');

            for (char* ptr = (char*) buf.read_ptr(); ptr != endptr; ptr++) {
                if (*ptr == '?') {
                    break;
                    // TODO handle arguments
                }

                req_path[req_path_len++] = *ptr;
            }
            req_path[req_path_len] = 0;
            buf.read_ack(req_path_len);
        }

        line_length += req_path_len;
    }
    // TODO handle headers & body

    while (*buf.read_ptr() != '\n') {
        buf.read_ack(1);
        line_length++;
    }

    // actually consume the '\n'
    buf.read_ack(1);
    line_length += 1;

    return line_length - 2; // minus \r\n
}

err_t HttpServerClient::flush() {
    return tcp_output(pcb);
}

err_t HttpServerClient::close() {
    err_t err = tcp_close(pcb);
    if (err != ERR_OK) {
        printf("close failed %d, calling abort\n", err);
        tcp_abort(pcb);
        return ERR_ABRT;
    }

    return err;
}

void HttpServerClient::send_string(const char* str) {
    int len = strlen(str);
    should_send += len;
    tcp_write(pcb, str, len, 0);
}

void HttpServerClient::response_begin(int code, const char* code_str) {
    char buf[128];

    snprintf(buf, 128, "HTTP/1.1 %d %s\r\n", code, code_str);
    send_string(buf);

    for (const auto& entry : headers) {
        snprintf(buf, 128, "%s: %s\r\n", entry.first.c_str(), entry.second.c_str());
        send_string(buf);
    }

    send_string("\r\n");
}

void HttpServerClient::not_found() {
    const char* msg = "<h2>Requested page not found</h2>";
    int len = strlen(msg);

    set_content_length(len);
    response_begin(404, "Not Found");
    send_string(msg);
}

void HttpServerClient::response_ok(const char* str) {
    set_content_length(strlen(str));
    response_begin(200, "OK");
    send_string(str);
}

void HttpServerClient::response_json(DynamicJsonDocument& json_doc, char* buf, int bufsize) {
    int len = serializeJson(json_doc, buf, bufsize);
    set_json();
    set_content_length(len);
    response_ok(buf);
}
