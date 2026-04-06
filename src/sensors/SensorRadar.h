#pragma once
#include <Arduino.h>

// ─── Capteur radar — détection de présence ───────────────
// Sortie logique HIGH = présence détectée.
class SensorRadar {
public:
    void begin(uint8_t pin);
    bool read();   // true = présence détectée

private:
    uint8_t _pin = 0;
};
