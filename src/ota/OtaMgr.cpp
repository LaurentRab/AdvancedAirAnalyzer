#include "OtaMgr.h"
#include "config.h"
#include <ArduinoOTA.h>
#include <Arduino.h>

void OtaMgr::begin() {
    ArduinoOTA.setHostname(DEVICE_NAME);
    ArduinoOTA.setPassword(OTA_PASSWORD);
    ArduinoOTA.setPort(OTA_PORT);

    ArduinoOTA.onStart([]() {
        const char* type = (ArduinoOTA.getCommand() == U_FLASH) ? "firmware" : "filesystem";
        Serial.printf("[OTA] Début — %s\n", type);
    });

    ArduinoOTA.onEnd([]() {
        Serial.println(F("\n[OTA] Terminé — redémarrage ..."));
    });

    ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
        Serial.printf("[OTA] %3u%%\r", progress * 100 / total);
    });

    ArduinoOTA.onError([](ota_error_t error) {
        Serial.printf("\n[OTA] Erreur [%u] : ", error);
        switch (error) {
            case OTA_AUTH_ERROR:    Serial.println(F("Authentification"));  break;
            case OTA_BEGIN_ERROR:   Serial.println(F("Begin"));             break;
            case OTA_CONNECT_ERROR: Serial.println(F("Connexion"));         break;
            case OTA_RECEIVE_ERROR: Serial.println(F("Réception"));         break;
            case OTA_END_ERROR:     Serial.println(F("Fin"));               break;
            default:                Serial.println(F("Inconnue"));          break;
        }
    });

    ArduinoOTA.begin();
    Serial.printf("[OTA] Prêt — hostname : %s, port : %d\n", DEVICE_NAME, OTA_PORT);
}

void OtaMgr::handle() {
    ArduinoOTA.handle();
}
