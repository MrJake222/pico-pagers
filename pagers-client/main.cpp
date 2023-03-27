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

    printf("Hello usb!\n");

    printf("CYCLE_TIME_US : %d\n", CYCLE_TIME_US);
    printf("HALF_CYCLE_TIME_US : %d\n", HALF_CYCLE_TIME_US);
    printf("SILENCE_LOW_TIME_US : %d\n", SILENCE_LOW_TIME_US);
    printf("SILENCE_HIGH_TIME_US : %d\n", SILENCE_HIGH_TIME_US);
    printf("ZERO_LOW_TIME_US : %d\n", ZERO_LOW_TIME_US);
    printf("ZERO_HIGH_TIME_US : %d\n", ZERO_HIGH_TIME_US);
    printf("ONE_LOW_TIME_US : %d\n", ONE_LOW_TIME_US);
    printf("ONE_HIGH_TIME_US : %d\n", ONE_HIGH_TIME_US);
    printf("ZERO_LOW_TIME_MARGIN : %d\n", ZERO_LOW_TIME_MARGIN);
    printf("ONE_LOW_TIME_MARGIN : %d\n", ONE_LOW_TIME_MARGIN);

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

    gpio_init(2);
    gpio_set_dir(2, GPIO_OUT);
    gpio_init(3);
    gpio_set_dir(3, GPIO_OUT);

    while (1) {

        uint8_t byte;

        int ret = receive_byte(&byte);
        if (ret == 0) {
            printf("rx: %02x\n", byte);
        }

        //printf("rx: %02x\n", byte);

        // TODO handle frame start detection
        /*uart_read_blocking(UART_ID, (uint8_t*)&frame, PROTO_DATA_SIZE);

        // TODO check return value
        proto_decrypt(public_key, &frame, &data);
        proto_checksum_verify(&data);

        // TODO verify sequence number
        printf("rid=%04x, seq=%16llx, type=%04x, param=%04x, checksum=%08x\n",
               data.receiver_id,
               data.sequence_number,
               data.message_type,
               data.message_param,
               data.checksum);

        sleep_ms(2000);*/


    }
}