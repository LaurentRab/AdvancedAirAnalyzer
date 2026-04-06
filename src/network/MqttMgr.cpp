#include "MqttMgr.h"
#include "config.h"
#include <Arduino.h>

MqttMgr::MqttMgr() : _client(_espClient) {}

void MqttMgr::begin() {
    _client.setServer(MQTT_SERVER, MQTT_PORT);
    _client.setKeepAlive(60);
    _connect();
}

bool MqttMgr::_connect() {
    if (_client.connected()) return true;
    Serial.println(F("[MQTT] Connexion ..."));
    const bool ok = _client.connect(
        MQTT_CLIENT_ID, MQTT_USER, MQTT_PASS,
        MQTT_TOPIC_STATUS, /*qos*/ 1, /*retain*/ true, "offline"
    );
    if (ok) {
        _client.publish(MQTT_TOPIC_STATUS, "online", /*retain*/ true);
        Serial.println(F("[MQTT] Connecte"));
    } else {
        Serial.printf("[MQTT] Echec, rc=%d\n", _client.state());
    }
    _lastAttemptMs = millis();
    return ok;
}

void MqttMgr::maintain() {
    if (_client.connected()) { _client.loop(); return; }
    if (millis() - _lastAttemptMs < MQTT_RECONNECT_MS) return;
    _connect();
}

bool MqttMgr::isConnected() { return _client.connected(); }

void MqttMgr::_pubFloat(const char* topic, float value, uint8_t decimals) {
    char buf[16];
    dtostrf(value, 0, decimals, buf);
    _client.publish(topic, buf, true);
}

void MqttMgr::_pubInt(const char* topic, int value) {
    char buf[12];
    snprintf(buf, sizeof(buf), "%d", value);
    _client.publish(topic, buf, true);
}

void MqttMgr::_pubULong(const char* topic, unsigned long value) {
    char buf[12];
    snprintf(buf, sizeof(buf), "%lu", value);
    _client.publish(topic, buf, true);
}


void MqttMgr::_pubStr(const char* topic, const char* value) {
    _client.publish(topic, value, true);
}

void MqttMgr::publish(const MeasurementData& m) {
    if (!_client.connected()) {
        Serial.println(F("[MQTT] Non connecte — publication ignoree"));
        return;
    }

    // SCD40 — CO2 + T/H diagnostiques (auto-échauffement visible ici)
    if (m.scdValid) {
        _pubInt  (MQTT_TOPIC_CO2,      (int)m.co2);
        _pubFloat(MQTT_TOPIC_TEMP_SCD, m.tempSCD);
        _pubFloat(MQTT_TOPIC_HUM_SCD,  m.humSCD);
    }

    // BME688 — T/H primaires + pression + gaz
    if (m.bmeValid) {
        _pubFloat(MQTT_TOPIC_TEMP,     m.temperature);   // référence ambiant
        _pubFloat(MQTT_TOPIC_HUM,      m.humidity);
        _pubFloat(MQTT_TOPIC_PRESSURE, m.pressure);
        _pubFloat(MQTT_TOPIC_GAS,      m.gasResistance / 1000.0f);
    }

    // PMS5003 — AE (terrain) + SP (référence)
    if (m.pmsValid) {
        _pubInt(MQTT_TOPIC_PM1_AE,  (int)m.pm1_ae);
        _pubInt(MQTT_TOPIC_PM25_AE, (int)m.pm25_ae);
        _pubInt(MQTT_TOPIC_PM10_AE, (int)m.pm10_ae);
        _pubInt(MQTT_TOPIC_PM1_SP,  (int)m.pm1_sp);
        _pubInt(MQTT_TOPIC_PM25_SP, (int)m.pm25_sp);
        _pubInt(MQTT_TOPIC_PM10_SP, (int)m.pm10_sp);
    }

    // LD2410C
    _pubStr(MQTT_TOPIC_PRESENCE, m.presence ? "ON" : "OFF");

    // Système
    _pubInt(MQTT_TOPIC_RSSI, m.rssi);
    _pubULong(MQTT_TOPIC_UPTIME, m.uptime);

    Serial.println(F("[MQTT] Publie"));
}

void MqttMgr::publishPresence(bool presence) {
    if (!_client.connected()) return;
    const char* payload = presence ? "ON" : "OFF";
    _client.publish(MQTT_TOPIC_PRESENCE, payload, true);
    Serial.printf("[MQTT] Presence : %s\n", payload);
}