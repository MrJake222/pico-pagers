#include "physical.hpp"

#include <pico/stdlib.h>
#include <cstdio>
#include <cstring>

#include "hardware/clocks.h"
#include "hardware/pwm.h"
#include "hardware/irq.h"

void config_print() {
    printf("physical interface config [us]:\n");
    printf("CYCLE_TIME_US       : %4d [us]\n", CYCLE_TIME_US);
    printf("PWM_TOP             : %4d\n", PWM_TOP);
    printf("PWM_DUTY_SILENCE    : %4d (%4d us high)\n", PWM_DUTY_SILENCE, CYCLE_TIME_US * PWM_DUTY_SILENCE  / PWM_TOP);
    printf("PWM_DUTY_ZERO       : %4d (%4d us high)\n", PWM_DUTY_ZERO,    CYCLE_TIME_US * PWM_DUTY_ZERO     / PWM_TOP);
    printf("PWM_DUTY_ONE        : %4d (%4d us high)\n", PWM_DUTY_ONE,     CYCLE_TIME_US * PWM_DUTY_ONE      / PWM_TOP);
    printf("zero is (%4d, %4d)\n", 0, PWM_DUTY_ZERO_MAX);
    printf(" one is (%4d, %4d)\n", PWM_DUTY_ONE_MIN, PWM_TOP);
    printf("SPACING_GENERATED_US    : %d\n", SPACING_GENERATED_US);
    printf("SPACING_ALLOWED_US_MAX  : %d\n", SPACING_ALLOWED_US_MAX);
}

// ------------------------------------ SENDING ------------------------------------ //

void pwm_wrap_irq();

uint slice_tx;

void send_setup() {
    gpio_set_function(PIN_TX, GPIO_FUNC_PWM);

    slice_tx = pwm_gpio_to_slice_num(PIN_TX);

    pwm_config c = pwm_get_default_config();
    pwm_config_set_wrap(&c, PWM_TOP);

    uint sysclk = clock_get_hz(clk_sys);
    float div = (float)sysclk / (float)(CLOCK_SPEED_HZ * PWM_TOP);
    pwm_config_set_clkdiv(&c, div);

    // If we ever want to use another PWM wrap handler, adjust this
    irq_set_exclusive_handler(PWM_IRQ_WRAP, pwm_wrap_irq);
    irq_set_enabled(PWM_IRQ_WRAP, true);
    pwm_set_irq_enabled(slice_tx, true);

    pwm_init(slice_tx, &c, false); // this zeroes out levels.
                                          // level setup needs to be lower.
    pwm_set_gpio_level(PIN_TX, PWM_DUTY_SILENCE);
    pwm_set_enabled(slice_tx, true);

    puts("PWM started");
}

volatile uint8_t tx_bytes[BYTE_BUFFER_SIZE];
volatile uint tx_byte_count;
volatile uint tx_bit_index;
volatile uint tx_byte_index;
volatile bool tx_transfer = false;

void pwm_wrap_irq() {
    if (pwm_get_irq_status_mask() & (1 << slice_tx)) {
        pwm_clear_irq(slice_tx);

        if (!tx_transfer) {
            return;
        }

        if (tx_bit_index == 8) {
            tx_bit_index = 0;
            tx_byte_index++;
            if (tx_byte_index == tx_byte_count) {
                tx_transfer = false;
                pwm_set_gpio_level(PIN_TX, PWM_DUTY_SILENCE);
                return;
            }
        }

        uint bit = (tx_bytes[tx_byte_index] << tx_bit_index) & 0x80;
        pwm_set_gpio_level(PIN_TX,
                           bit ? PWM_DUTY_ONE : PWM_DUTY_ZERO);
        tx_bit_index++;
    }
}

void send_bytes(uint8_t* bytes, int count) {
    // wait for end of previous transfer
    while (tx_transfer);
    sleep_us(SPACING_GENERATED_US);

    // start new transfer
    memcpy((void*)tx_bytes, (void*)bytes, count);
    tx_byte_count = count;
    tx_bit_index = 0;
    tx_byte_index = 0;
    tx_transfer = true;
}


// ------------------------------------ RECEIVING ------------------------------------ //

uint slice_rx;

void rx_fall_callback(uint gpio, uint32_t events);

void receive_setup() {
    // gpio_init(PIN_RX);
    gpio_set_dir(PIN_RX, GPIO_IN);
    gpio_set_irq_enabled_with_callback(PIN_RX, GPIO_IRQ_EDGE_FALL, true, rx_fall_callback);

    gpio_set_function(PIN_RX, GPIO_FUNC_PWM);

    slice_rx = pwm_gpio_to_slice_num(PIN_RX);

    pwm_config c = pwm_get_default_config();
    pwm_config_set_wrap(&c, PWM_TOP);

    uint sysclk = clock_get_hz(clk_sys);
    float div = (float)sysclk / (float)(CLOCK_SPEED_HZ * PWM_TOP);
    pwm_config_set_clkdiv(&c, div);
    pwm_config_set_clkdiv_mode(&c, PWM_DIV_B_HIGH);

    pwm_init(slice_rx, &c, true);
    puts("PWM started");
}

uint64_t last_good_bit;

volatile uint8_t rx_bytes[BYTE_BUFFER_SIZE];
volatile uint rx_bit_index;
volatile uint rx_byte_index;

RxCallback cb;

void rx_fall_callback(uint gpio, uint32_t events) {
    gpio_acknowledge_irq(gpio, events);
    uint cnt = pwm_get_counter(slice_rx);
    pwm_set_counter(slice_rx, 0);

    uint64_t now = time_us_64();

    int bit;
    if (cnt < PWM_DUTY_ZERO_MAX) {
        bit = 0;
    }
    else if (cnt > PWM_DUTY_ONE_MIN) {
        bit = 1;
    }
    else {
        // silence
        bit = -1;
        if (now - last_good_bit > SPACING_ALLOWED_US_MAX) {
            // end of frame
            if (rx_byte_index > 0)
                cb(rx_bytes, rx_byte_index);

            rx_byte_index = 0;
            rx_bit_index = 0;
        }
    }

    if (bit != -1) {
        last_good_bit = now;
        // printf("cnt: %4d -> bit %2d\n", cnt, bit);

        rx_bytes[rx_byte_index] <<= 1;
        rx_bytes[rx_byte_index] ^= (-bit ^ rx_bytes[rx_byte_index]) & 1UL;

        rx_bit_index++;
        if (rx_bit_index == 8) {
            rx_bit_index = 0;
            rx_byte_index++;
            if (rx_byte_index == BYTE_BUFFER_SIZE) {
                puts("discarding data, buffer overflow");
                rx_byte_index = 0;
            }
        }
    }
}

void rx_set_callback(RxCallback cb_) {
    cb = cb_;
}