#include <stdio.h>
#include <pico/stdlib.h>
#include <hardware/uart.h>

#include "physical.hpp"

// hardware UART on physical pins
#define UART_ID uart0
#define BAUD_RATE 9600
#define UART_TX_PIN 0
#define UART_RX_PIN 1

#include <protocol.hpp>

int main() {

    // UART on USB
    stdio_usb_init();

    sleep_ms(2000);
    printf("\n\nHello usb pagers-server!\n");
    config_print();

    // hardware UART
    /*uart_init(UART_ID, BAUD_RATE);
    gpio_set_function(UART_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(UART_RX_PIN, GPIO_FUNC_UART);*/

    // printf writes to USB only
    // uart_* write to pin GP(UART_TX_PIN) only

    struct proto_data data = {
            .receiver_id = 0x1215,
            .sequence_number = 0xCAFEDEADBEEFCAFE,
            .message_type = 0xDBDB,
            .message_param = 0xACDC
    };

    const uint8_t private_key[KEY_LENGTH_BYTES] = { 0 };
    struct proto_frame frame;

    send_setup();

    gpio_init(2);
    gpio_set_dir(2, GPIO_OUT);

    uint8_t val = 0;

    while (1) {

        /*printf("sizeof short %u\n", sizeof(short));
        printf("sizeof int %u, sizeof long %u, sizeof long long %u\n", sizeof(int), sizeof(long), sizeof(long long));
        printf("sizeof proto data %u\n", PROTO_DATA_SIZE);

        data.sequence_number++;

        proto_checksum_calc(&data);
        proto_encrypt(private_key, &data, &frame);

        uart_write_blocking(UART_ID, (uint8_t*)&frame, PROTO_DATA_SIZE);
        sleep_ms(2000);*/


        // delay 1s
        // 1ms / 1000us = 1
        // 1s / 1000us = 1000
        // 10ms / 1000us = 10
        int cnt = 1000;
        while (cnt--) send_silence();

        // TODO remove GP2 (used for testing)
        gpio_put(2, 1);

        send_byte(val++);

        gpio_put(2, 0);
    }
}