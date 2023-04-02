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

const int LED_RED = 15;
const int LED_YELLOW = 14;
const int LED_GREEN = 13;

int main() {

    // UART on USB
    stdio_usb_init();

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
    gpio_init(2); gpio_set_dir(2, GPIO_OUT);
    gpio_init(3); gpio_set_dir(3, GPIO_OUT);

    // status leds
    gpio_init(LED_RED);    gpio_set_dir(LED_RED, GPIO_OUT);
    gpio_init(LED_YELLOW); gpio_set_dir(LED_YELLOW, GPIO_OUT);
    gpio_init(LED_GREEN);  gpio_set_dir(LED_GREEN, GPIO_OUT);
    gpio_put(LED_RED, true);
    gpio_put(LED_YELLOW, true);
    gpio_put(LED_GREEN, true);
    sleep_ms(500);
    gpio_put(LED_RED, false);
    gpio_put(LED_YELLOW, false);
    gpio_put(LED_GREEN, false);
    sleep_ms(1500);

    printf("\n\nHello usb pagers-client!\n");
    config_print();

    while (1) {
        int err = receive_bytes((uint8_t*)&frame, PROTO_FRAME_SIZE);
        if (err != 0) {
            // error
            continue;
        }

        // TODO check return value
        proto_decrypt(public_key, &frame, &data);
        int checksum_verify = proto_checksum_verify(&data);

        if (checksum_verify == SUCCESS) {
            gpio_put(LED_GREEN, true);
            sleep_ms(50);
            gpio_put(LED_GREEN, false);
        } else {
            gpio_put(LED_RED, true);
            sleep_ms(50);
            gpio_put(LED_RED, false);
        }

        // TODO verify sequence number

        int time_seconds = time_us_64() / 1000000;
        printf("[%02d:%02d] rid=%04x, seq=%16llx, type=%04x, param=%04x, checksum=%04x [%s]\n",
               time_seconds / 60, time_seconds % 60,
               data.receiver_id,
               data.sequence_number,
               data.message_type,
               data.message_param,
               data.checksum,
               checksum_verify == SUCCESS ? "ok" : "wrong checksum");
    }
}