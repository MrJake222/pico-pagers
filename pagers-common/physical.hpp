#pragma once

#include <cstdint>

// settings
const int CLOCK_SPEED_HZ = 6400;
const int PIN_TX = 0;
const int PIN_RX = 1;


// private defines
const int CYCLE_TIME_US = 1000000 / CLOCK_SPEED_HZ;
const int HALF_CYCLE_TIME_US = CYCLE_TIME_US / 2;

// tx
const int SILENCE_LOW_TIME_US = HALF_CYCLE_TIME_US;
const int SILENCE_HIGH_TIME_US = HALF_CYCLE_TIME_US;
const int ZERO_LOW_TIME_US = CYCLE_TIME_US * 4 / 5;
const int ZERO_HIGH_TIME_US = CYCLE_TIME_US * 1 / 5;
const int ONE_LOW_TIME_US = CYCLE_TIME_US * 1 / 5;
const int ONE_HIGH_TIME_US = CYCLE_TIME_US * 4 / 5;

// rx
const int ZERO_LOW_TIME_MARGIN = (ZERO_LOW_TIME_US + SILENCE_LOW_TIME_US) / 2;
const int ONE_LOW_TIME_MARGIN = (ONE_LOW_TIME_US + SILENCE_LOW_TIME_US) / 2;

// TODO make a class for different pins
// TODO refactor this into interrupts

// config
void config_print();

// sending
void send_setup();
void send_bit(bool bit);
void send_silence();
void send_byte(uint8_t byte);
void send_bytes(uint8_t* bytes, int count);

// receiving
// returns -1 on silence received, 0 on success
void receive_setup();
int receive_bit(bool* bit);
int receive_byte(uint8_t* byte);
int receive_bytes(uint8_t* bytes, int count);
