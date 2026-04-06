#include "SensorRadar.h"

void SensorRadar::begin(uint8_t pin) {
    _pin = pin;
    pinMode(_pin, INPUT);
    Serial.println(F("[Radar] OK"));
}

bool SensorRadar::read() {
    return digitalRead(_pin) == HIGH;
}
