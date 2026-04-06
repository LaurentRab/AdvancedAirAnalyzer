#pragma once
#include <Arduino.h>

// ─── LD2410C — Radar de détection de présence ─────────────────────────────
//
// Connexion actuelle : sortie logique OUT sur PIN_RADAR (GPIO 10)
//   OUT = HIGH → présence détectée (cible mobile OU statique)
//   OUT = LOW  → aucune présence
//
// Note : le LD2410C dispose d'un protocole UART riche permettant d'obtenir :
//   - distinction cible mobile / cible statique
//   - distance détectée (cm) pour chaque type de cible
//   - niveau d'énergie pour chaque porte de détection
//   Si vous câblez TX/RX du radar sur l'ESP32, utilisez la bibliothèque
//   « ncmreynolds/ld2410 » et remplacez cette classe par SensorLD2410C_UART.
//
class SensorLD2410C {
public:
    // pin : broche OUT du LD2410C (sortie logique de présence)
    void begin(uint8_t pin);
    bool read();   // true = présence détectée

private:
    uint8_t _pin = 0;
};
