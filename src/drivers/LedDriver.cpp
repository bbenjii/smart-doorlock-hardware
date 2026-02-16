#include "drivers/LedDriver.h"

LedDriver::LedDriver(uint8_t pin) : _pin(pin) {}

void LedDriver::begin() {
    pinMode(_pin, OUTPUT);
    digitalWrite(_pin, LOW);
}

void LedDriver::on() {
    digitalWrite(_pin, HIGH);
}

void LedDriver::off() {
    digitalWrite(_pin, LOW);
}
