#pragma once
#include <HardwareSerial.h>
#include <PMS.h>

// ─── PMS5003 — particules fines ───────────────────────────────────────────
//
// Gestion optimisée du ventilateur (composant mécanique sensible) :
//   La broche SET (GPIO PIN_PMS_SET) coupe physiquement le ventilateur
//   et le laser quand elle est LOW. Durée de vie typique : 8 000 h.
//   En ne l'activant que 30 s toutes les 15 min, on réduit l'usure de ~97%.
//
//   begin()   : configure SET LOW (veille hardware immédiate)
//   wakeUp()  : SET HIGH → wakeUp UART → passiveMode (ordre important)
//   read()    : requestRead() + readUntil() en mode passif
//   sleep()   : sleep UART → délai → SET LOW (extinction hardware)
//
// Données exposées :
//   AE (Atmospheric Environment) — valeurs terrain, recommandées pour IAQ
//   SP (Standard Particles CF=1) — valeurs de référence pour comparaisons
//
class SensorPMS5003 {
public:
    struct Data {
        // Concentration massique atmosphérique (µg/m³) — usage terrain
        uint16_t pm1_ae  = 0;
        uint16_t pm25_ae = 0;
        uint16_t pm10_ae = 0;
        // Concentration massique standard CF=1 (µg/m³) — référence labo
        uint16_t pm1_sp  = 0;
        uint16_t pm25_sp = 0;
        uint16_t pm10_sp = 0;
        bool     valid   = false;
    };

    // rxPin, txPin : UART vers PMS5003
    // setPin       : broche SET — LOW=veille, HIGH=actif
    bool begin(uint8_t rxPin, uint8_t txPin, uint8_t setPin);
    void wakeUp();
    void sleep();
    Data read(uint32_t timeoutMs = 5000);

private:
    HardwareSerial _serial{0};   // UART0 : RX=20, TX=21 sur ESP32-C3
    PMS*           _pms    = nullptr;
    uint8_t        _setPin = 0;

    void _flushSerial();   // purge le buffer UART (trames mode actif résiduelles)
};
