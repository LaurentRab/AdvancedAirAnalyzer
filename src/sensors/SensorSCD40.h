#pragma once
#include <SensirionI2CScd4x.h>

// ─── SCD40 — CO2, température, humidité ──────────────────
// Le capteur tourne en mode « periodic measurement » (1 mesure/5 s).
// read() vérifie la disponibilité des données avant de lire.
class SensorSCD40 {
public:
    struct Data {
        uint16_t co2         = 0;
        float    temperature = 0.0f;
        float    humidity    = 0.0f;
        bool     valid       = false;
    };

    bool begin();
    Data read();

private:
    SensirionI2CScd4x _scd4x;
};
