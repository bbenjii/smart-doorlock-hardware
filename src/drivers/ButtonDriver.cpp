#include "drivers/ButtonDriver.h"

#define SHORT_PRESS_TIME 1000 // 1 second
#define LONG_PRESS_TIME  3000 // 5 seconds


ButtonDriver::ButtonDriver(uint8_t pin) : _pin(pin) {}

void ButtonDriver::begin() {
    pinMode(_pin, INPUT_PULLUP);
}

bool ButtonDriver:: isPressed() {
    return digitalRead(_pin) == HIGH;
}

bool ButtonDriver::detect() {
    bool longDetected = false;
    currentState = digitalRead(_pin);

    if (lastState == HIGH && currentState == LOW) {      // button is pressed
        pressedTime = millis();
        isPressing = true;
        isLongDetected = false;


    }
    else if (lastState == LOW && currentState == HIGH) { // button is released
        isPressing = false;

        releasedTime = millis();

        long pressDuration = releasedTime - pressedTime;

        if ( pressDuration < SHORT_PRESS_TIME ) Serial.println("A short press is detected");
    }

    if (isPressing == true && isLongDetected == false) {
    long pressDuration = millis() - pressedTime;

    if ( pressDuration > LONG_PRESS_TIME ) {
      Serial.println("A long press is detected");
      isLongDetected = true;
      longDetected = true;
    }
  }

    // save the the last state
    lastState = currentState;
    return longDetected;
}   
