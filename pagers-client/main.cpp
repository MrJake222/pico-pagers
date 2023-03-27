#include <stdio.h>
#include <pico/stdlib.h>
#include <hardware/uart.h>

// hardware UART on physical pins
#define UART_ID uart0
#define BAUD_RATE 9600
#define UART_TX_PIN 0
#define UART_RX_PIN 1

#include <protocol.hpp>

int main() {

    // UART on USB
    stdio_usb_init();
    printf("Hello usb!\n");

    // hardware UART
    uart_init(UART_ID, BAUD_RATE);
    gpio_set_function(UART_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(UART_RX_PIN, GPIO_FUNC_UART);

    // printf writes to USB only
    // uart_* write to pin GP(UART_TX_PIN) only

    struct proto_frame frame;
    struct proto_data data;

    const uint8_t public_key[KEY_LENGTH_BYTES] = { 0 };

    while (1) {

        // TODO handle frame start detection
        uart_read_blocking(UART_ID, (uint8_t*)&frame, sizeof(struct proto_frame));

        // TODO check return value
        proto_decrypt(public_key, &frame, &data);
        proto_checksum_verify(&data);

        // TODO verify sequence number
        // TODO print data

        sleep_ms(2000);
    }
}