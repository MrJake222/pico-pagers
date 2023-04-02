#include "physical.hpp"

#include <pico/stdlib.h>
#include <cstdio>

void config_print() {
    printf("physical interface config:\n");
    printf("CYCLE_TIME_US : %d\n", CYCLE_TIME_US);
    printf("HALF_CYCLE_TIME_US : %d\n", HALF_CYCLE_TIME_US);
    printf("SILENCE_LOW_TIME_US : %d\n", SILENCE_LOW_TIME_US);
    printf("SILENCE_HIGH_TIME_US : %d\n", SILENCE_HIGH_TIME_US);
    printf("ZERO_LOW_TIME_US : %d\n", ZERO_LOW_TIME_US);
    printf("ZERO_HIGH_TIME_US : %d\n", ZERO_HIGH_TIME_US);
    printf("ONE_LOW_TIME_US : %d\n", ONE_LOW_TIME_US);
    printf("ONE_HIGH_TIME_US : %d\n", ONE_HIGH_TIME_US);
    printf("ZERO_LOW_TIME_MARGIN : %d\n", ZERO_LOW_TIME_MARGIN);
    printf("ONE_LOW_TIME_MARGIN : %d\n\n", ONE_LOW_TIME_MARGIN);
}

// ------------------------------------ SENDING ------------------------------------ //

void send_setup() {
    gpio_init(PIN_TX);
    gpio_set_dir(PIN_TX, GPIO_OUT);
}

void send_bit(bool bit) {
    if (bit) {
        gpio_put(PIN_TX, false);
        sleep_us(ONE_LOW_TIME_US);
        gpio_put(PIN_TX, true);
        sleep_us(ONE_HIGH_TIME_US);
    }
    else {
        gpio_put(PIN_TX, false);
        sleep_us(ZERO_LOW_TIME_US);
        gpio_put(PIN_TX, true);
        sleep_us(ZERO_HIGH_TIME_US);
    }
}

void send_silence() {
    gpio_put(PIN_TX, false);
    sleep_us(SILENCE_LOW_TIME_US);
    gpio_put(PIN_TX, true);
    sleep_us(SILENCE_HIGH_TIME_US);
}

void send_byte(uint8_t byte) {

    for (int i=0; i<8; i++) {
        send_bit(byte & 0x80);
        byte <<= 1;
    }
}

void send_bytes(uint8_t* bytes, int count) {
    while (count--) {
        send_byte(*bytes++);
    }
}


// ------------------------------------ RECEIVING ------------------------------------ //

void receive_setup() {
    gpio_init(PIN_RX);
    gpio_set_dir(PIN_RX, GPIO_IN);
}

int receive_bit(uint8_t* bit) {
    // wait for falling
    while (gpio_get(PIN_RX) != 0);

    // wait for rising + measure time of low state
    uint64_t time_start = time_us_64();
    while (gpio_get(PIN_RX) == 0);

    int us = (int)(time_us_64() - time_start);

    // zero for <us> ms
    if (us < ONE_LOW_TIME_MARGIN) {
        *bit = 1;
    }
    else if (us < ZERO_LOW_TIME_MARGIN) {
        // silence
        return -1;
    }
    else {
        *bit = 0;
    }

    return 0;
}

int receive_byte(uint8_t *byte) {

    *byte = 0;

    for (int i=0; i<8; i++) {
        uint8_t bit;
        int err = receive_bit(&bit);
        if (err) {
            return err;
        }

        *byte <<= 1;
        *byte += bit;
    }

    return 0;
}

int receive_bytes(uint8_t* bytes, int count) {

    uint64_t last_good_byte = time_us_64();

    while (count) {
        // intentionally wait for all bytes, ignoring short interruptions
        int err = receive_byte(bytes);
        if (err == 0) {
            count--;
            bytes++; // advance pointer
            last_good_byte = time_us_64();
        }
        else {
            // silence detected
            if (time_us_64() - last_good_byte > MAX_SILENCE_ALLOWED) {
                // silence too long
                return -1;
            }
            // if not too long -> ignore for now
        }
    }

    return 0;
}
