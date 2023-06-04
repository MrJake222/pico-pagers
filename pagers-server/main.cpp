#include <stdio.h>
#include <pico/stdlib.h>

#include <lfs.h>
#include <fs.hpp>

#include <pico/cyw43_arch.h>

#include "dhcpserver.h"
#include "dnsserver.h"

#include "physical.hpp"
#include <protocol.hpp>

lfs_t lfs;
lfs_file_t file;

#include "http/http.hpp"

int main() {

    // UART on USB
    // stdio_usb_init();
    // UART on all
    stdio_init_all();

    // sleep_ms(2000);
    printf("\n\nHello usb pagers-server!\n");

    // Initialise the access point

    TCP_SERVER_T *state = static_cast<TCP_SERVER_T *>(calloc(1, sizeof(TCP_SERVER_T)));
    if (!state) {
        printf("failed to allocate state\n");
        return 1;
    }

    printf("cyw43 initialization...\n");
    if (cyw43_arch_init_with_country(CYW43_COUNTRY_POLAND)) {
        printf("cy43 init failed\n");
        return 1;
    };
    printf("cyw43 initialised\n");

    cyw43_arch_enable_ap_mode(ap_name, ap_password, CYW43_AUTH_WPA2_AES_PSK);

    ip4_addr_t mask;
    IP4_ADDR(ip_2_ip4(&state->gw), 192, 168, 4, 1);
    IP4_ADDR(ip_2_ip4(&mask), 255, 255, 255, 0);

    // Start the dhcp server
    dhcp_server_t dhcp_server;
    dhcp_server_init(&dhcp_server, &state->gw, &mask);

    // Start the dns server
    dns_server_t dns_server;
    dns_server_init(&dns_server, &state->gw);

    // Open the TCP server
    if (!tcp_server_open(state)) {
        printf("failed to open server\n");
        return 1;
    }

    /**
     *
     * REST OF THE CODE
     *
     */

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
        printf("running\n");


        data.sequence_number++;

        proto_checksum_calc(&data);
        proto_encrypt(private_key, &data, &frame);

        send_bytes((uint8_t*)&frame, PROTO_FRAME_SIZE);
    }
}