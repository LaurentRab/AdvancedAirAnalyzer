#include "SensorBME688.h"
#include <Arduino.h>

bool SensorBME688::begin(uint8_t addr) {
    _bme.begin(addr, Wire);
    if (_bme.checkStatus() == BME68X_ERROR) {
        Serial.printf("[BME688] Init error: %s\n", _bme.statusString());
        return false;
    }
    _bme.setTPH();                  // oversampling par défaut (T×2, P×16, H×1)
    _bme.setHeaterProf(300, 100);   // chauffage gaz : 300 °C / 100 ms
    _ok = true;
    Serial.println(F("[BME688] OK"));
    return true;
}

SensorBME688::Data SensorBME688::read() {
    Data d;
    if (!_ok) return d;

    // Déclenche une mesure unique (forced mode)
    _bme.setOpMode(BME68X_FORCED_MODE);

    // Pattern correct pour cette bibliothèque (v1.1.x) :
    //   fetchData() interroge le registre du capteur et remplit le cache interne.
    //   getData()   lit seulement ce cache — inutile sans fetchData() préalable.
    //
    // Polling jusqu'à ce que fetchData() retourne nFields > 0 (mesure prête),
    // ou timeout 500 ms pour ne pas bloquer le watchdog.
    uint8_t nFields = 0;
    const unsigned long deadline = millis() + 500UL;
    while (nFields == 0 && millis() < deadline) {
        delay(10);
        nFields = _bme.fetchData();
    }

    if (nFields == 0) {
        Serial.println(F("[BME688] Timeout — aucune donnee recue"));
        return d;
    }

    bme68xData raw;
    _bme.getData(raw);

    d.temperature   = raw.temperature;
    d.humidity      = raw.humidity;
    d.pressure      = raw.pressure / 100.0f;   // Pa → hPa
    d.gasResistance = raw.gas_resistance;
    d.valid         = true;
    return d;
}
