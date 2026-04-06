#include "SensorLD2410C.h"

void SensorLD2410C::begin(uint8_t pin) {
    _pin = pin;
    pinMode(_pin, INPUT);
    Serial.printf("[LD2410C] OK — OUT pin=%d (detection presence)\n", _pin);
}

bool SensorLD2410C::read() {
    return digitalRead(_pin) == HIGH;
}
