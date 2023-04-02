#include "protocol.hpp"

#include <cstring>
#include "crc.hpp"

int proto_checksum_calc(struct proto_data *data) {
    data->checksum = crc_16((unsigned char*)data, PROTO_DATA_SIZE - PROTO_CHECKSUM_SIZE);
    return SUCCESS;
}

int proto_checksum_verify(struct proto_data *data) {
    unsigned short crc_should_be = crc_16((unsigned char*)data, PROTO_DATA_SIZE - PROTO_CHECKSUM_SIZE);
    if (crc_should_be != data->checksum)
        return ERR_WRONG_CHECKSUM;

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
