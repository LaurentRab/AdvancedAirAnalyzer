#include "SensorSCD40.h"
#include "config.h"
#include <Arduino.h>

bool SensorSCD40::begin() {
    _scd4x.begin(Wire);

    // Si le capteur est déjà en mode périodique (boot précédent),
    // stopPeriodicMeasurement() le remet en idle.
    // La datasheet impose 500 ms d'attente après ce stop.
    _scd4x.stopPeriodicMeasurement();
    delay(500);

    // Compense l'auto-échauffement interne du SCD40.
    // Sans cet offset, le SCD40 surestime la température ambiante,
    // ce qui dégrade la précision du calcul CO2 (qui dépend de T et H).
    // Valeur configurée dans config.h (SCD40_TEMP_OFFSET).
    _scd4x.setTemperatureOffset(SCD40_TEMP_OFFSET);

    uint16_t err = _scd4x.startPeriodicMeasurement();
    if (err) {
        Serial.printf("[SCD40] startPeriodicMeasurement error: %u\n", err);
        return false;
    }
    Serial.printf("[SCD40] OK — temp offset=%.1f C\n", SCD40_TEMP_OFFSET);
    return true;
}

SensorSCD40::Data SensorSCD40::read() {
    Data d;

    bool isReady = false;
    uint16_t err = _scd4x.getDataReadyFlag(isReady);
    if (err) {
        Serial.printf("[SCD40] getDataReadyFlag error: %u\n", err);
        return d;
    }
    if (!isReady) {
        Serial.println(F("[SCD40] Data not ready"));
        return d;
    }

    err = _scd4x.readMeasurement(d.co2, d.temperature, d.humidity);
    if (err) {
        Serial.printf("[SCD40] readMeasurement error: %u\n", err);
        return d;
    }
    if (d.co2 == 0) {
        Serial.println(F("[SCD40] Invalid sample (co2=0)"));
        return d;
    }

    d.valid = true;
    return d;
}
