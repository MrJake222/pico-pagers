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
    if (strncmp((char*)buf.read_ptr(), "GET", 3) == 0) {
        // GET method
        buf.read_ack(3+1); // move read ptr past GET+space
        line_length += 4;
        method = Method::GET;
    }
    else {
        method = Method::INVALID;
    }

    int req_path_len = 0;
    if (method != Method::INVALID) {
        // find space after request path
        char* endptr = strchr((char*)buf.read_ptr(), ' ');

        for (char* ptr=(char*)buf.read_ptr(); ptr!=endptr; ptr++) {
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

    while (*buf.read_ptr() != '\n') {
        buf.read_ack(1);
        line_length++;
    }

    // actually consume the '\n'
    buf.read_ack(1);
    line_length += 1;

    return line_length - 2; // minus \r\n
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


