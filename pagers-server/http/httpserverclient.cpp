#include <cstring>
#include "httpserverclient.hpp"

static const char* strchrto(const char* from, const char* to, char chr) {

    for (; from<to; from++) {
        if (*from == chr)
            break;
    }

    return from;
}

static char name[128];
static char val[128];

int HttpServerClient::parse_url_arguments(const char* ptr) {
    if (!*ptr)
        return 0;

    char* endptr;

    const char* start = ptr;
    const char* end;

    while (true) {
        end = strchrto(start, start + strlen(start), '&');
        const char* eq = strchrto(start, end, '=');
        endptr = stpncpy(name, start, eq - start);
        *endptr = '\0';

        if (eq != end) {
            // value present
            eq++; // skip =
            endptr = stpncpy(val, eq, end - eq);
            *endptr = '\0';
        } else {
            val[0] = 0;
        }

        request_params[name] = val;

        if (!*end)
            break;
        start = end + 1;
    }

    return end - ptr;
}

int HttpServerClient::parse_header(const char* ptr) {
    if (*ptr == '\r')
        // empty line
        return 0;

    const char* start = ptr;

    const char* sep = strchr(start, ':');
    char* endptr = stpncpy(name, start, sep - start);
    *endptr = '\0';

    sep += 1; // skip :
    if (*sep == ' ')
        sep += 1; // skip ' '

    char* end = strchr(sep, '\r');
    endptr = stpncpy(val, sep, end - sep);
    *endptr = '\0';

    request_headers[name] = val;

    return end - ptr;
}

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
        } else if (strncmp((char*) buf.read_ptr(), "POST", 4) == 0) {
            // GET method
            buf.read_ack(4 + 1); // move read ptr past GET+space
            line_length += 5;
            method = Method::POST;
        } else {
            method = Method::INVALID;
        }

        int req_path_len = 0;
        if (method != Method::INVALID) {
            // find space after request path
            char* endptr = strchr((char*) buf.read_ptr(), ' ');
            *endptr = '\0'; // null terminate

            for (const char* ptr = (char*) buf.read_ptr(); ptr != endptr;) {
                if (*ptr == '?') {
                    ptr += 1; // consume "?"
                    int consumed = parse_url_arguments(ptr);
                    ptr += consumed;
                    line_length += consumed;
                    buf.read_ack(consumed);
                }
                else {
                    req_path[req_path_len++] = *ptr++;
                }
            }
            req_path[req_path_len] = 0;
            buf.read_ack(req_path_len);
        }

        line_length += req_path_len;
    }
    else {
        // not first line, headers
        int consumed = parse_header((char*) buf.read_ptr());
        line_length += consumed;
        buf.read_ack(consumed);
    }

    while (*buf.read_ptr() != '\n') {
        buf.read_ack(1);
        line_length++;
    }

    // actually consume the '\n'
    buf.read_ack(1);
    line_length += 1;

    return line_length;
}

void HttpServerClient::handle_body() {
    if (!has_req_header("Content-Length"))
        return; // no content length

    char* endptr;
    int len = strtol(get_req_header("Content-Length").c_str(), &endptr, 10);
    if (*endptr)
        return; // not null terminated

    if (!has_req_header("Content-Type"))
        return; // no content type

    // TODO handle different content-type's
    if (get_req_header("Content-Type") == "application/x-www-form-urlencoded") {
        char* ptr = (char*) buf.read_ptr();
        ptr[len] = '\0';

        int consumed = parse_url_arguments(ptr);
        buf.read_ack(consumed);
    }
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

    for (const auto& entry : response_headers) {
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
