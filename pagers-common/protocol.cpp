#include "protocol.hpp"

#include <cstring>

int proto_checksum_calc(struct proto_data *data) {
    data->checksum = 0;
    return SUCCESS;
}

int proto_checksum_verify(struct proto_data *data) {
    return SUCCESS;
}

int proto_encrypt(const uint8_t *private_key, const struct proto_data *data, struct proto_frame *frame) {
    // TODO secure this
    ::memcpy(frame->encrypted_data, data, PROTO_DATA_SIZE);

    return SUCCESS;
}

int proto_decrypt(const uint8_t *public_key, const struct proto_frame *frame, struct proto_data *data) {
    // TODO secure this
    ::memcpy(data, frame->encrypted_data, PROTO_DATA_SIZE);

    return SUCCESS;
}
