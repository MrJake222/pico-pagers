#pragma once

#include <cstdint>
#include <pico/platform.h>

// settings

// max bytes to send in one send_bytes call
#define BYTE_BUFFER_SIZE 128

const int CLOCK_SPEED_HZ = 2400; // anything beyond 2400 (or 4800 with tuning) loses impulses (see docs/img/bauds)
const int PIN_TX = 15;
const int PIN_RX = 17; // must be odd (PWM channel B)


// private defines
const int CYCLE_TIME_US = 1000000 / CLOCK_SPEED_HZ;

// Config
const int SUB_CYCLES = 6;
const int SUB_CYCLES_HIGH_SILENCE = 3;
// transmitter
const int SUB_CYCLES_HIGH_ZERO = 1;
const int SUB_CYCLES_HIGH_ONE = 5;
// receiver allowed
const int SUB_CYCLES_HIGH_ZERO_MAX = 2;
const int SUB_CYCLES_HIGH_ONE_MIN = 4;

// PWM
const int PWM_TOP = 1024;
const int PWM_DUTY_SILENCE = PWM_TOP * SUB_CYCLES_HIGH_SILENCE / SUB_CYCLES;
// transmitter
const int PWM_DUTY_ZERO = PWM_TOP * SUB_CYCLES_HIGH_ZERO / SUB_CYCLES;
const int PWM_DUTY_ONE = PWM_TOP * SUB_CYCLES_HIGH_ONE / SUB_CYCLES;
// receiver allowed
const int PWM_DUTY_ZERO_MAX = PWM_TOP * SUB_CYCLES_HIGH_ZERO_MAX / SUB_CYCLES;
const int PWM_DUTY_ONE_MIN = PWM_TOP * SUB_CYCLES_HIGH_ONE_MIN / SUB_CYCLES;

// transfers spacing
const int SPACING_GENERATED_US   = CYCLE_TIME_US * 20; // char times
const int SPACING_ALLOWED_US_MAX = CYCLE_TIME_US * 10; // char times


// TODO make a class for different pins

// config
void config_print();

// sending
void send_setup();
// this function is non-blocking, it starts a transfer
// and returns
void send_bytes(uint8_t* bytes, int count);
void send_wait_for_end();

// receiving
typedef void(*RxCallback)(const volatile uint8_t* buf, volatile uint bytes);
void receive_setup();
void rx_set_callback(RxCallback cb_);
