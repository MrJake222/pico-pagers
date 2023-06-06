#include <stdio.h>
#include <pico/stdlib.h>

#include <lfs.h>
#include <fs.hpp>

#include <pico/cyw43_arch.h>

#include "dhcpserver.h"
#include "dnsserver.h"

#include "physical.hpp"
#include <protocol.hpp>
#include <vector>

lfs_t lfs;
lfs_file_t file;

#include "httpserver.hpp"
#include "ArduinoJson.h"

char jbuf[8*1024];
DynamicJsonDocument json(8*1024);

void json_test_page(HttpServerClient* client, void* arg) {
    json.clear();
    json["val"] = 123;
    json["key"] = "value";
    client->response_json(json, jbuf, 8*1024);
}

void form_test_page(HttpServerClient* client, void* arg) {
    puts("headers:");
    for (auto h : client->get_req_headers()) {
        printf("\t%s: %s\n", h.first.c_str(), h.second.c_str());
    }
    puts("headers end");

    puts("params:");
    for (auto p : client->get_req_params()) {
        printf("\t%s: %s\n", p.first.c_str(), p.second.c_str());
    }
    puts("end params");

    client->response_ok("ok");
}


/* ------------------------------------------- WIFI ------------------------------------------- */

volatile bool wifi_scan = false;
typedef std::map<std::string, cyw43_ev_scan_result_t> ScanResultsMap;
ScanResultsMap* scan_results = nullptr;

void http_wifi_scan_start(HttpServerClient* client, void* arg) {
    scan_results = new ScanResultsMap;
    wifi_scan = true;
    client->response_ok("ok");
}

void http_wifi_scan_status(HttpServerClient* client, void* arg) {
    json.clear();
    json["active"] = cyw43_wifi_scan_active(&cyw43_state);
    client->response_json(json, jbuf, 8*1024);
}

void http_wifi_scan_results(HttpServerClient* client, void* arg) {
    if (!scan_results) {
        client->response_bad("start a scan first");
        return;
    }

    if (cyw43_wifi_scan_active(&cyw43_state)) {
        client->response_bad("scan still in progress");
        return;
    }

    json.clear();

    for (auto r : *scan_results) {
        json[r.first]["ssid"]    = r.second.ssid;
        json[r.first]["rssi"]    = r.second.rssi;
        json[r.first]["channel"] = r.second.channel;
        json[r.first]["auth"]    = r.second.auth_mode;
    }

    delete scan_results;
    scan_results = nullptr;

    client->response_json(json, jbuf, 8*1024);
}

int scan_result(void* env, const cyw43_ev_scan_result_t* result) {
    if (result && scan_results) {
        (*scan_results)[(char*)result->ssid] = *result;

        printf("ssid: %-32s rssi: %4d chan: %3d mac: %02x:%02x:%02x:%02x:%02x:%02x sec: %u\n",
               result->ssid, result->rssi, result->channel,
               result->bssid[0], result->bssid[1], result->bssid[2], result->bssid[3], result->bssid[4], result->bssid[5],
               result->auth_mode);
    }
    return 0;
}

void do_wifi_scan() {
    cyw43_arch_enable_sta_mode();

    cyw43_wifi_scan_options_t scan_options = {0};

    int err = cyw43_wifi_scan(&cyw43_state, &scan_options, nullptr, scan_result);
    if (err) {
        printf("Failed to start scan err=%d\n", err);
    }
    else {
        puts("WiFi scan started");
    }
}


/* -------------------------------------------  ------------------------------------------- */
/* -------------------------------------------  ------------------------------------------- */
/* -------------------------------------------  ------------------------------------------- */
/* -------------------------------------------  ------------------------------------------- */
/* -------------------------------------------  ------------------------------------------- */

int main() {

    // UART on USB
    // stdio_usb_init();
    // UART on all
    stdio_init_all();

    sleep_ms(2000);
    printf("\n\nHello usb pagers-server!\n");
    config_print();

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
    server.set_cb_arg(nullptr);
    server.start(80);
    server.static_content(&lfs, "/static");
    server.on(Method::GET, "/json", json_test_page);
    server.on(Method::GET, "/form", form_test_page);
    server.on(Method::POST, "/form", form_test_page);

    /* WiFI */
    server.on(Method::GET, "/wifi/scan/start", http_wifi_scan_start);
    server.on(Method::GET, "/wifi/scan/status", http_wifi_scan_status);
    server.on(Method::GET, "/wifi/scan/results", http_wifi_scan_results);

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
        // puts("running");
        sleep_ms(250);

        data.sequence_number++;

        proto_checksum_calc(&data);
        proto_encrypt(private_key, &data, &frame);

        send_bytes((uint8_t*)&frame, PROTO_FRAME_SIZE);
        send_wait_for_end();

        if (wifi_scan) {
            wifi_scan = false;
            do_wifi_scan();
        }
    }
}