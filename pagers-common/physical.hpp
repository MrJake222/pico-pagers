#pragma once

#include <cstdint>

// settings

// max bytes to send in one send_bytes call
#define BYTE_BUFFER_SIZE 128

const int CLOCK_SPEED_HZ = 1000; // anything beyond this loses packets (todo refactor receiver)
const int PIN_TX = 15;
const int PIN_RX = 16;


// private defines
const int CYCLE_TIME_US = 1000000 / CLOCK_SPEED_HZ;

// Config
const int SUB_CYCLES = 6;
const int SUB_CYCLES_HIGH_SILENCE = 3;
const int SUB_CYCLES_HIGH_ZERO = 1;
const int SUB_CYCLES_HIGH_ONE = 5;

// PWM
const int PWM_TOP = 1024;
const int PWM_DUTY_SILENCE = PWM_TOP * SUB_CYCLES_HIGH_SILENCE / SUB_CYCLES;
const int PWM_DUTY_ZERO = PWM_TOP * SUB_CYCLES_HIGH_ZERO / SUB_CYCLES;
const int PWM_DUTY_ONE = PWM_TOP * SUB_CYCLES_HIGH_ONE / SUB_CYCLES;

// transfers spacing
const int MAX_SILENCE_ALLOWED_US = CYCLE_TIME_US * 10; // char times
const int MIN_SILENCE_GENERATED_US = MAX_SILENCE_ALLOWED_US * 2; // char times


// legacy
const int SUB_CYCLE_TIME_US = CYCLE_TIME_US / SUB_CYCLES;
// tx
const int SILENCE_LOW_TIME_US = SUB_CYCLE_TIME_US * (SUB_CYCLES - SUB_CYCLES_HIGH_SILENCE);
const int SILENCE_HIGH_TIME_US = SUB_CYCLE_TIME_US * SUB_CYCLES_HIGH_SILENCE;
const int ZERO_LOW_TIME_US = SUB_CYCLE_TIME_US * (SUB_CYCLES - SUB_CYCLES_HIGH_ZERO);
const int ZERO_HIGH_TIME_US = SUB_CYCLE_TIME_US * SUB_CYCLES_HIGH_ZERO;
const int ONE_LOW_TIME_US = SUB_CYCLE_TIME_US * (SUB_CYCLES - SUB_CYCLES_HIGH_ONE);
const int ONE_HIGH_TIME_US = SUB_CYCLE_TIME_US * SUB_CYCLES_HIGH_ONE;
// rx
const int ZERO_LOW_TIME_MARGIN = (ZERO_LOW_TIME_US + SILENCE_LOW_TIME_US) / 2;
const int ONE_LOW_TIME_MARGIN = (ONE_LOW_TIME_US + SILENCE_LOW_TIME_US) / 2;

// TODO make a class for different pins
// TODO refactor this into interrupts

// config
void config_print();

// sending
void send_setup();
// this function is non-blocking, it starts a transfer
// and returns
void send_bytes(uint8_t* bytes, int count);
void send_wait_for_end();

// receiving
// returns -1 on silence received, 0 on success
void receive_setup();
int receive_bit(bool* bit);
int receive_byte(uint8_t* byte);
int receive_bytes(uint8_t* bytes, int count);
