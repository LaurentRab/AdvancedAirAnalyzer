#include "SensorPMS5003.h"
#include <Arduino.h>

bool SensorPMS5003::begin(uint8_t rxPin, uint8_t txPin, uint8_t setPin) {
    _setPin = setPin;
    
    // 1. Initialiser la broche SET immédiatement pour réveiller le hardware
    pinMode(_setPin, OUTPUT);
    digitalWrite(_setPin, HIGH); 
    delay(200); // Laisser l'IC du PMS démarrer

    // 2. Initialiser l'UART
    _serial.begin(9600, SERIAL_8N1, rxPin, txPin);
    _pms = new PMS(_serial);

    // 3. Forcer le mode passif et purger
    _pms->passiveMode();
    delay(100);
    _flushSerial();

    // 4. Maintenant seulement, mettre en veille si nécessaire
    // Ou mieux : le laisser tourner pour le premier test
    return true;
}

void SensorPMS5003::wakeUp() {
    pinMode(_setPin, OUTPUT);
    digitalWrite(_setPin, HIGH);

    // 1. Délai allongé (500ms) pour laisser l'électronique du capteur démarrer proprement
    delay(500);

    // 2. Forcer le réveil logiciel (annule tout état de veille fantôme d'un ancien firmware)
    _pms->wakeUp();
    delay(50);

    _pms->passiveMode();   // le capteur redémarre en mode actif — repasser en passif
    _flushSerial();
    Serial.printf("[PMS5003] Reveil — SET pin %d HIGH, mode passif\n", _setPin);
}

void SensorPMS5003::sleep() {
    // Coupure hardware uniquement.
    // Pas de commande UART sleep() : elle laisserait le PMS dans un état
    // "double veille" (UART sleep + SET LOW) qui perturbe le prochain réveil.
    // La broche SET suffit : SET LOW coupe ventilateur et laser immédiatement.
    digitalWrite(_setPin, LOW);
    Serial.println(F("[PMS5003] Veille (SET LOW) — ventilateur arrete"));
}

SensorPMS5003::Data SensorPMS5003::read(uint32_t timeoutMs) {
    Data d;
    if (!_pms) return d;

    // 1. On purge le buffer des vieilles trames accumulées pendant le warmup
    _flushSerial();

    // 2. En mode passif, il faut demander explicitement une trame
    _pms->requestRead();

    // 3. Chronomètre pour le timeout
    uint32_t start = millis();
    PMS::DATA raw;

    Serial.println(F("[PMS5003] Lecture..."));

    // 4. On boucle tant qu'on n'a pas dépassé le timeout
    while (millis() - start < timeoutMs) {
        // .read(raw) renvoie true dès qu'une trame complète (32 octets) 
        // est décodée avec succès depuis le flux série
        if (_pms->read(raw)) {
            // Sanity check : un échantillon avec toutes les valeurs de particules
            // à zéro est très improbable dans un environnement normal et indique
            // souvent que le capteur n'est pas encore stabilisé.
            if (raw.PM_AE_UG_1_0 == 0 && raw.PM_AE_UG_2_5 == 0 && raw.PM_AE_UG_10_0 == 0) {
                Serial.println(F("[PMS5003] Echantillon invalide (valeurs nulles) ignoré."));
                // En mode passif, une seule trame est envoyée par requête.
                // On ne peut pas attendre une autre trame, donc on abandonne cette lecture.
                return d; // d.valid est toujours false.
            }
            d.pm1_ae  = raw.PM_AE_UG_1_0;
            d.pm25_ae = raw.PM_AE_UG_2_5;
            d.pm10_ae = raw.PM_AE_UG_10_0;
            d.pm1_sp  = raw.PM_SP_UG_1_0;
            d.pm25_sp = raw.PM_SP_UG_2_5;
            d.pm10_sp = raw.PM_SP_UG_10_0;
            d.valid   = true;
            Serial.printf("[PMS5003] PM1: %d | PM2.5: %d | PM10: %d ug/m3\n",
                          d.pm1_ae, d.pm25_ae, d.pm10_ae);
            return d;
        }
        yield(); // Laisse l'ESP respirer (gestion WiFi/WDT)
    }

    Serial.println(F("[PMS5003] Timeout : Aucune trame valide reçue."));
    return d;
}

void SensorPMS5003::_flushSerial() {
    while (_serial.available()) _serial.read();
}
