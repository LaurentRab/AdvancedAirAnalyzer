#include "WiFiMgr.h"
#include "config.h"
#include <Arduino.h>

void WiFiMgr::connect() {
    _doConnect();
}

void WiFiMgr::_doConnect() {
    Serial.printf("[WiFi] Connexion à %s ...\n", WIFI_SSID);
    WiFi.mode(WIFI_STA);
    WiFi.setAutoReconnect(false);   // géré manuellement ici
    WiFi.setSleep(false);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

    const unsigned long start = millis();
    while (WiFi.status() != WL_CONNECTED &&
           millis() - start < WIFI_TIMEOUT_MS) {
        delay(500);
        Serial.print('.');
    }
    Serial.println();

    if (WiFi.status() == WL_CONNECTED) {
        Serial.printf("[WiFi] Connecté — IP : %s | RSSI : %d dBm\n",
                      WiFi.localIP().toString().c_str(), WiFi.RSSI());
    } else {
        Serial.println(F("[WiFi] Échec de connexion"));
    }
    _lastAttemptMs = millis();
}

void WiFiMgr::maintain() {
    if (WiFi.status() == WL_CONNECTED) return;
    if (millis() - _lastAttemptMs < MQTT_RECONNECT_MS) return;
    Serial.println(F("[WiFi] Reconnexion ..."));
    _doConnect();
}

bool WiFiMgr::isConnected() const {
    return WiFi.status() == WL_CONNECTED;
}

int WiFiMgr::rssi() const {
    return WiFi.RSSI();
}
