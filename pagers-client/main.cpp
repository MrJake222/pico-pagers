#include <stdio.h>
#include <pico/stdlib.h>

#include <protocol.hpp>
#include <physical.hpp>
#include <cstring>

const int BUTTON = 10;

const int LED_RED = 15;
const int LED_YELLOW = 14;
const int LED_GREEN = 13;

volatile bool frame_present;
volatile struct proto_frame frame;

bool previous_button_state = false;
bool is_pairing_mode = false;

void frame_received(const volatile uint8_t* buf, uint bytes) {
    if (!frame_present) {
        if (bytes != PROTO_DATA_SIZE) {
            printf("wrong number of bytes to decode, is %d, should be %d: ", bytes, PROTO_DATA_SIZE);
            for (int i=0; i<bytes; i++)
                printf("%02x ", buf[i]);
            puts("");
            return;
        }

        memcpy((void*) &frame, (const void*) buf, PROTO_DATA_SIZE);
        frame_present = true;
    }
}


// device id
unsigned short device_id = 0x1215;
const uint8_t public_key[KEY_LENGTH_BYTES] = { 0 };
struct proto_data data;

int main() {

    // UART on USB
    // printf writes to USB only
    stdio_usb_init();

    // pair button
    gpio_init(BUTTON);
    gpio_set_dir(BUTTON, GPIO_IN);

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

    frame_present = false;

    receive_setup();
    rx_set_callback(frame_received);

    while (1) {
        if (gpio_get(BUTTON)) {
            if (!previous_button_state) {
                printf("button pressed\n");
                previous_button_state = true;
                is_pairing_mode = !is_pairing_mode;
                printf("pairing mode %d\n", is_pairing_mode);
            }
        } else {
            previous_button_state = false;
        }
        if (frame_present) {
            // TODO check return value
            proto_decrypt(public_key, (struct proto_frame*)&frame, &data);
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

            if (data.message_type == MessageType::PAIR) {
                printf("Received pairing message!\n\n\n");
                is_pairing_mode = false;
                device_id = data.receiver_id;
            }

            if (data.receiver_id == device_id) {
                printf("Received message for me!\n\n\n");
            }

            int time_seconds = time_us_64() / 1000000;
            printf("[%02d:%02d] rid=%04x, seq=%16llx, type=%04x, param=%04x, checksum=%04x [%s]\n",
                   time_seconds / 60, time_seconds % 60,
                   data.receiver_id,
                   data.sequence_number,
                   data.message_type,
                   data.message_param,
                   data.checksum,
                   checksum_verify == SUCCESS ? "ok" : "wrong checksum");

            // set to false after the frame has been processed
            frame_present = false;
        }
    }
}