#include "physical.hpp"

#include <pico/stdlib.h>
#include <cstdio>
#include <cstring>

#include "hardware/clocks.h"
#include "hardware/pwm.h"
#include "hardware/irq.h"

void config_print() {
    printf("physical interface config:\n");
    printf("CYCLE_TIME_US : %d\n", CYCLE_TIME_US);
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

void pwm_wrap_irq();

uint slice_num;

void send_setup() {
    gpio_set_function(PIN_TX, GPIO_FUNC_PWM);

    slice_num = pwm_gpio_to_slice_num(PIN_TX);

    pwm_config c = pwm_get_default_config();
    pwm_config_set_wrap(&c, PWM_TOP);

    uint sysclk = clock_get_hz(clk_sys);
    float div = (float)sysclk / (float)(CLOCK_SPEED_HZ * PWM_TOP);
    pwm_config_set_clkdiv(&c, div);

    // If we ever want to use another PWM wrap handler, adjust this
    irq_set_exclusive_handler(PWM_IRQ_WRAP, pwm_wrap_irq);
    irq_set_enabled(PWM_IRQ_WRAP, true);
    pwm_set_irq_enabled(slice_num, true);

    pwm_set_gpio_level(PIN_TX, PWM_DUTY_SILENCE);
    pwm_init(slice_num, &c, true);

    puts("PWM started");
}

volatile uint8_t bytes_to_send[BYTE_BUFFER_SIZE];
volatile uint byte_count;
volatile uint bit_index;
volatile uint byte_index;
volatile bool transfer = false;

void pwm_wrap_irq() {
    if (pwm_get_irq_status_mask() & (1<<slice_num)) {
        pwm_clear_irq(slice_num);

        if (!transfer) {
            return;
        }

        if (bit_index == 8) {
            bit_index = 0;
            byte_index++;
            if (byte_index == byte_count) {
                transfer = false;
                pwm_set_gpio_level(PIN_TX, PWM_DUTY_SILENCE);
                return;
            }
        }

        uint bit = (bytes_to_send[byte_index] << bit_index) & 0x80;
        pwm_set_gpio_level(PIN_TX,
                           bit ? PWM_DUTY_ONE : PWM_DUTY_ZERO);
        bit_index++;
    }
}

void send_bytes(uint8_t* bytes, int count) {
    memcpy((void*)bytes_to_send, (void*)bytes, count);
    byte_count = count;
    bit_index = 0;
    byte_index = 0;
    transfer = true;
}

void send_wait_for_end() {
    while (transfer);
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
            if (time_us_64() - last_good_byte > MAX_SILENCE_ALLOWED_US) {
                // silence too long
                return -1;
            }
            // if not too long -> ignore for now
        }
    }

    return 0;
}
