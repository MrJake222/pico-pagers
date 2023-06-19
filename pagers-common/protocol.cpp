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

int proto_encrypt(const struct proto_data *data, struct proto_frame *frame) {
    crypto_encrypt(PROTO_DATA_SIZE / 2, (const ushort*)data, (uint*)frame->encrypted_data);

    return SUCCESS;
}

int proto_decrypt(const struct proto_frame *frame, struct proto_data *data) {
    crypto_decrypt(PROTO_DATA_SIZE / 2, (ushort*)data, (uint*)frame->encrypted_data);

    return SUCCESS;
}
