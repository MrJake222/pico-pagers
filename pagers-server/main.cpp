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

// #include "http/http.hpp"
#include "httpserver.hpp"
#include "ArduinoJson.h"

void root(HttpServerClient* client, void* arg) {
    client->response_ok("<h1>OK</h1>");
}

char jbuf[8*1024];
DynamicJsonDocument json(8*1024);

void json_test_page(HttpServerClient* client, void* arg) {
    json.clear();
    json["val"] = 123;
    json["key"] = "value";
    client->response_json(json, jbuf, 8*1024);
}

int main() {

    // UART on USB
    // stdio_usb_init();
    // UART on all
    stdio_init_all();

    // sleep_ms(2000);
    printf("\n\nHello usb pagers-server!\n");

    int r = lfs_mount(&lfs, &pico_lfs_config);
    if (r < 0) {
        printf("failed to mount fs err=%d\n", r);
        while(1);
    }

    puts("fs mount ok");

    // Initialise the access point
    printf("cyw43 initialization...\n");
    if (cyw43_arch_init_with_country(CYW43_COUNTRY_POLAND)) {
        printf("cy43 init failed\n");
        return 1;
    };
    printf("cyw43 initialised\n");

    char ap_name[] = "pagers-server";
    char ap_password[] = "password";

    cyw43_arch_enable_ap_mode(ap_name, ap_password, CYW43_AUTH_WPA2_AES_PSK);
    puts("ap init ok");


    ip4_addr_t gw;
    ip4_addr_t mask;
    IP4_ADDR(ip_2_ip4(&gw), 192, 168, 4, 1);
    IP4_ADDR(ip_2_ip4(&mask), 255, 255, 255, 0);


    // Start the dhcp server
    dhcp_server_t dhcp_server;
    dhcp_server_init(&dhcp_server, &gw, &mask);
    puts("dhcp server started");


    // Start the dns server
    dns_server_t dns_server;
    dns_server_init(&dns_server, &gw);
    puts("dns server started");


    // Open the TCP server
    HttpServer server;
    server.start(80);
    server.static_content(&lfs, "/static");
    server.on(Method::GET, "/root", root);
    server.on(Method::GET, "/json", json_test_page);

    /**
     *
     * REST OF THE CODE
     *
     */


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