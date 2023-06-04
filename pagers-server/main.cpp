#include <stdio.h>
#include <pico/stdlib.h>

#include <lfs.h>
#include <fs.hpp>

#include "physical.hpp"
#include <protocol.hpp>

// variables used by the filesystem
lfs_t lfs;
lfs_file_t file;

int main() {

    // UART on USB
    // stdio_usb_init();
    // UART on all
    stdio_init_all();

    // sleep_ms(2000);
    printf("\n\nHello usb pagers-server!\n");
    config_print();

    int r = lfs_mount(&lfs, &pico_lfs_config);
    if (r < 0) {
        printf("failed to mount fs err=%d\n", r);
        while(1);
    }

    puts("fs mount ok");

    lfs_dir_t dir;
    r = lfs_dir_open(&lfs, &dir, "/");
    if (r < 0) {
        printf("failed to open root dir err=%d\n", r);
        while(1);
    }

    puts("listing files:");
    lfs_info info;
    while (1) {
        r = lfs_dir_read(&lfs, &dir, &info);
        if (r < 0) {
            printf("failed to read root dir err=%d\n", r);
            while(1);
        }
        if (r == 0) {
            // end of dir
            break;
        }

        // filled
        printf("\t%s\n", info.name);
    }

    puts("done");


    struct proto_data data = {
            .sequence_number = 0, //0xCAFEDEADBEEFCAFE,
            .receiver_id = 0x1215,
            .message_type = 0xDBDB,
            .message_param = 0xACDC
    };

    const uint8_t private_key[KEY_LENGTH_BYTES] = { 0 };
    struct proto_frame frame;

    send_setup();

    // for testing
    gpio_init(2);
    gpio_set_dir(2, GPIO_OUT);

    proto_checksum_calc(&data);
    proto_encrypt(private_key, &data, &frame);
    printf("sending: ");
    for (int i=0; i<PROTO_DATA_SIZE; i++)
        printf("%02x ", ((uint8_t*)&frame)[i]);
    printf("\n");

    printf("sizeof proto_data %u\n", sizeof(struct proto_data));
    printf("sizeof proto_frame %u\n", sizeof(struct proto_frame));

    while (1) {
        int cnt = CLOCK_SPEED_HZ * 1; // delay in seconds
        while (cnt--) send_silence();


        data.sequence_number++;

        proto_checksum_calc(&data);
        proto_encrypt(private_key, &data, &frame);

        send_bytes((uint8_t*)&frame, PROTO_FRAME_SIZE);
    }
}