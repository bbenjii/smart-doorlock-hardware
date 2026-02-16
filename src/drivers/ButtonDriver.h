#pragma once
#include <Arduino.h>

class ButtonDriver {
    public:
        explicit ButtonDriver(uint8_t pin);

        void begin();
        // bool isPressed();

        bool isPressed();

        bool detect(); // returns true when a long press is detected
    
    private:
        uint8_t _pin;
        int lastState = LOW; // the previous state from the input pin
        int currentState; // the current reading from the input pin
        unsigned long pressedTime  = 0;
        unsigned long releasedTime = 0;
        bool isPressing = false;
        bool isLongDetected = false;


};
