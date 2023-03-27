#include "physical.hpp"

#include <pico/stdlib.h>

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


// ------------------------------------ RECEIVING ------------------------------------ //

void receive_setup() {
    gpio_init(PIN_RX);
    gpio_set_dir(PIN_RX, GPIO_IN);
}

int receive_bit(uint8_t* bit) {
    // wait for falling
    while (gpio_get(PIN_RX) != 0);

    // wait for rising
    int us = 0;
    while (gpio_get(PIN_RX) == 0) {
        // TODO extremely hacky, anecdotal evidence, refactor ASAP
        // experiments proves sleep_us(1) actually takes approx 2.6 us
        // this should be an interrupt
        // TODO remove GP2 (used for testing only)
        gpio_put(2, 1);
        sleep_us(1);
        us+=3;
        gpio_put(2, 0);
        sleep_us(1);
        us+=3;
    }

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
