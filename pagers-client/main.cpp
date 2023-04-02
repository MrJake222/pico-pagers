#include <stdio.h>
#include <pico/stdlib.h>
#include <hardware/uart.h>

// hardware UART on physical pins
#define UART_ID uart0
#define BAUD_RATE 9600
#define UART_TX_PIN 0
#define UART_RX_PIN 1

#include <protocol.hpp>
#include <physical.hpp>

int main() {

    // UART on USB
    stdio_usb_init();

    sleep_ms(2000);
    printf("\n\nHello usb pagers-client!\n");
    config_print();

    // hardware UART
    /*uart_init(UART_ID, BAUD_RATE);
    gpio_set_function(UART_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(UART_RX_PIN, GPIO_FUNC_UART);*/

    // printf writes to USB only
    // uart_* write to pin GP(UART_TX_PIN) only

    struct proto_frame frame;
    struct proto_data data;

    const uint8_t public_key[KEY_LENGTH_BYTES] = { 0 };

    receive_setup();

    // for testing
    gpio_init(2);
    gpio_set_dir(2, GPIO_OUT);
    gpio_init(3);
    gpio_set_dir(3, GPIO_OUT);

    while (1) {
        int err = receive_bytes((uint8_t*)&frame, PROTO_FRAME_SIZE);
        if (err != 0) {
            // error
            continue;
        }

        // TODO check return value
        proto_decrypt(public_key, &frame, &data);
        proto_checksum_verify(&data);

        int time_seconds = time_us_64() / 1000000;

        // TODO verify sequence number
        printf("[%02d:%02d] rid=%04x, seq=%16llx, type=%04x, param=%04x, checksum=%08x\n",
               time_seconds / 60, time_seconds % 60,
               data.receiver_id,
               data.sequence_number,
               data.message_type,
               data.message_param,
               data.checksum);

        sleep_ms(2000);
    }
}