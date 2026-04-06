#pragma once
#include <bme68xLibrary.h>

// ─── BME688 — pression, température ambiante, humidité, qualité gaz ──
// Utilisé en mode « forced » : une mesure déclenchée à la demande.
class SensorBME688 {
public:
    struct Data {
        float temperature   = 0.0f;   // °C
        float humidity      = 0.0f;   // %RH
        float pressure      = 0.0f;   // hPa
        float gasResistance = 0.0f;   // Ω  (diviser par 1000 pour kΩ)
        bool  valid         = false;
    };

    bool begin(uint8_t addr = 0x76);
    Data read();

private:
    Bme68x _bme;
    bool   _ok = false;
};
