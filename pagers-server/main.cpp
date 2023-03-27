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

    /*uint8_t data[10] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
    size_t datalen = 10;*/

    struct proto_data data = {
            .receiver_id = 1,
            .sequence_number = 1,
            .message_type = 1,
            .message_param = 1
    };

    const uint8_t private_key[KEY_LENGTH_BYTES] = { 0 };
    struct proto_frame frame;

    while (1) {

        data.sequence_number++;

        proto_checksum_calc(&data);
        proto_encrypt(private_key, &data, &frame);

        uart_write_blocking(UART_ID, (uint8_t*)&frame, sizeof(struct proto_frame));

        sleep_ms(2000);
    }
}