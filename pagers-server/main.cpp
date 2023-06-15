#include <stdio.h>
#include <pico/stdlib.h>

#include <lfs.h>
#include <fs.hpp>
#include <wififs.hpp>

#include <pico/cyw43_arch.h>

#include "dhcpserver.h"
#include "dnsserver.h"

#include "physical.hpp"
#include <protocol.hpp>
#include <vector>
#include <tuple>

lfs_t lfs;

#include "httpserver.hpp"
#include "ArduinoJson.h"
#include "pagers/pagerlist.hpp"

char jbuf[8*1024];
DynamicJsonDocument json(8*1024);

const unsigned short DEFAULT_FLASH_MSGS = 5;

unsigned long long sequence_number = 0;

PagerList pagers(&lfs, "/pagers");


/* ------------------------------------------- WiFi Scan ------------------------------------------- */

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


/* ------------------------------------------- WiFi connect ------------------------------------------- */

uint32_t fix_auth(uint8_t sec) {
    if (sec & 0x04) {
        return CYW43_AUTH_WPA2_AES_PSK;
    }

    if (sec & 0x02) {
        return CYW43_AUTH_WPA_TKIP_PSK;
    }

    // if (sec & 0x01) {
    //     // wep
    //     return -1;
    // }

    return -1;
}

volatile bool wifi_connect = false;
volatile char connect_ssid[32];
volatile char connect_pw[64];
volatile uint32_t connect_auth;
volatile int connect_err_code;

void do_wifi_connect(bool save = true) {
    cyw43_arch_enable_sta_mode();

    printf("connecting to wifi ssid='%s' pwd='%s' auth=0x%08lx\n", connect_ssid, connect_pw, connect_auth);

    int err = cyw43_arch_wifi_connect_blocking(
            (char*)connect_ssid,
            (char*)connect_pw,
            connect_auth);

    connect_err_code = err;

    if (err) {
        printf("Failed to connect err=%d\n", err);
    }
    else {
        puts("connected");
        if (save)
            wifi_save(&lfs, (const char*)connect_ssid, (const char*)connect_pw, (uint32_t)connect_auth);
    }
}

void http_wifi_connect(HttpServerClient* client, void* arg) {
    if (!client->has_req_param("ssid") || !client->has_req_param("password") || !client->has_req_param("auth")) {
        client->response_bad("no ssid or password or auth given");
        return;
    }

    strncpy((char*)connect_ssid, client->get_req_param("ssid").c_str(),     32);
    strncpy((char*)connect_pw,   client->get_req_param("password").c_str(), 64);
    connect_auth = fix_auth(client->get_req_param_int("auth"));
    wifi_connect = true;

    client->response_ok("ok");
}

void http_wifi_connect_status(HttpServerClient* client, void* arg) {
    json.clear();

    json["success"] = (connect_err_code == 0);
    json["err_code"] = connect_err_code;

    cyw43_arch_lwip_begin();
    const ip4_addr_t* addr;
    addr = netif_ip4_addr(&cyw43_state.netif[CYW43_ITF_STA]);
    cyw43_arch_lwip_end();

    json["ip"] = ip4addr_ntoa(addr);

    client->response_json(json, jbuf, 8*1024);
}

/* ------------------------------------------- Pagers pairing ------------------------------------------- */

unsigned short pairing_device_id = -1;
int pairing_messages_left = 0;

void send_message(struct proto_data* data) {
    sequence_number++;
    data->sequence_number = sequence_number;

    struct proto_frame frame;
    proto_checksum_calc(data);
    proto_encrypt(data, &frame);

    send_bytes((uint8_t*)&frame, PROTO_FRAME_SIZE);
}

void send_pairing_message() {
    struct proto_data data = {
            .receiver_id = pairing_device_id,
            .message_type = MessageType::PAIR,
            .message_param = 0x0,
    };
    send_message(&data);

    printf("sending pairing message, pager id=%d, left=%d\n", pairing_device_id, pairing_messages_left-1);
}

void http_pagers_pair(HttpServerClient* client, void* arg) {
    if (!client->has_req_param("id")) {
        client->response_bad("no pager id given");
        return;
    }

    uint32_t id = client->get_req_param_int("id");
    pairing_device_id = id;

    if (pagers.pager_exists(id)) {
        client->response_ok("pager already exist... remove old pager");
        return;
    }

    pairing_messages_left = 10;
    auto pager = new Pager(pairing_device_id);
    pagers.add_pager(pager);

    printf("added pager, total: %d\n", pagers.size());

    client->response_ok("started pairing...");
}

/* ------------------------------------------- Pagers management ------------------------------------------- */

void http_pagers_list(HttpServerClient* client, void* arg) {
    json.clear();

    json["total_pagers"] = pagers.size();

    for (auto& r : pagers) {
        auto pager = r.second;
        printf("Pager list: %04x\n", pager->get_device_id());
        json["pagers"][std::to_string(pager->get_device_id())] = 30; // TODO why
    }

    client->response_json(json, jbuf, 8*1024);
}

void http_pagers_remove(HttpServerClient* client, void* arg) {
    if (!client->has_req_param("id")) {
        client->response_bad("no pager id given");
        return;
    }

    unsigned short device_id = client->get_req_param_int("id");

    delete pagers.get_pager(device_id); // free memory
    bool res = pagers.remove_pager(device_id);

    client->response_ok(res ? "removed" : "pager not found");
}

void http_pagers_flash(HttpServerClient* client, void* arg) {
    if (!client->has_req_param("id")) {
        client->response_bad("no pager id given");
        return;
    }

    if (!client->has_req_param("time")) {
        client->response_bad("no pager flash time given");
        return;
    }

    unsigned short device_id = client->get_req_param_int("id");
    auto pager = pagers.get_pager(device_id);
    pager->set_flash_msgs_left(DEFAULT_FLASH_MSGS);
    pager->flash_time = client->get_req_param_int("time");

    client->response_ok("flashing the pager");
}

void send_flash_messages() {
    for (auto pair : pagers) {
        auto pager = pair.second;
        if (pager->any_flash_msgs_left()) {
            struct proto_data data = {
                    .receiver_id = pager->get_device_id(),
                    .message_type = MessageType::FLASH,
                    .message_param = pager->flash_time, // sets timer in client
            };
            send_message(&data);
            pager->flash_msg_sent();
        }
    }
}

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

    pagers.load_fs();
    puts("pagers load finished");

    // Initialise the access point
    printf("cyw43 initialization...\n");
    if (cyw43_arch_init_with_country(CYW43_COUNTRY_POLAND)) {
        printf("cy43 init failed\n");
        return 1;
    };
    printf("cyw43 initialised\n");

    r = wifi_read(&lfs, (char*)connect_ssid, (char*)connect_pw, (uint32_t*)&connect_auth);
    if (r < 0) {
        // failed to read config
        // enable AP mode
        char ap_name[] = "pagers-server";
        char ap_password[] = "password";

        puts("ap init start");
        cyw43_arch_enable_ap_mode(ap_name, ap_password, CYW43_AUTH_WPA2_AES_PSK);
        puts("ap init ok");
    }
    else {
        // read ok
        printf("connect to %s\n", connect_ssid);
        do_wifi_connect(false);
    }


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

    /* WiFI scan */
    server.on(Method::GET, "/wifi/scan/start", http_wifi_scan_start);
    server.on(Method::GET, "/wifi/scan/status", http_wifi_scan_status);
    server.on(Method::GET, "/wifi/scan/results", http_wifi_scan_results);
    /* WiFI connect */
    server.on(Method::GET, "/wifi/connect", http_wifi_connect);
    server.on(Method::GET, "/wifi/connect/status", http_wifi_connect_status);

    /* Pair new pager */
    server.on(Method::GET, "/pagers/pair", http_pagers_pair);
    server.on(Method::GET, "/pagers/list", http_pagers_list);
    server.on(Method::GET, "/pagers/remove", http_pagers_remove);
    server.on(Method::GET, "/pagers/flash", http_pagers_flash);

    send_setup();

    while (1) {
        sleep_ms(250);

        if (wifi_scan) {
            wifi_scan = false;
            do_wifi_scan();
        }

        if (wifi_connect) {
            wifi_connect = false;
            do_wifi_connect();
        }

        if (pairing_messages_left > 0) {
            send_pairing_message();
            pairing_messages_left--;
            sleep_ms(50);
        }

        send_flash_messages();

        server.loop();
        pagers.loop();
    }
}