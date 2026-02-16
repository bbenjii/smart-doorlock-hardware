#pragma once
#include <Arduino.h>

class LedDriver {
    public:
        explicit LedDriver(uint8_t pin);

        void begin();
        void on();
        void off();

    private:
        uint8_t _pin;
};
