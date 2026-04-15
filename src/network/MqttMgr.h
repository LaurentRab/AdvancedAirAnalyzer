#pragma once
#include <PubSubClient.h>
#include <WiFiClient.h>

// ─── Structure agrégée de toutes les mesures ─────────────────────────────
// Découple la couche réseau des modules capteurs.
struct MeasurementData {
    // SCD40
    uint16_t co2      = 0;
    float    tempSCD  = 0.0f;
    float    humSCD   = 0.0f;
    bool     scdValid = false;

    // BME688 (BSEC2)
    float temperature   = 0.0f;
    float humidity      = 0.0f;
    float pressure      = 0.0f;
    float gasResistance = 0.0f;
    float iaq           = 0.0f;    // IAQ Bosch (0-500)
    int   iaqAccuracy   = 0;      // 0-3
    float co2Equiv      = 0.0f;    // eCO2 Bosch
    float breathVoc     = 0.0f;
    bool  bmeValid      = false;

    // PMS5003 + LD2410C + Système
    uint16_t pm1_ae = 0, pm25_ae = 0, pm10_ae = 0;
    uint16_t pm1_sp = 0, pm25_sp = 0, pm10_sp = 0;
    bool     pmsValid = false;
    bool     presence = false;
    float    healthScore = 0.0f;   // Index de confinement calculé (0-100%)
    int      rssi = 0;
    unsigned long uptime = 0;
};

// ─── Gestionnaire MQTT ────────────────────────────────────────────────────
class MqttMgr {
public:
    MqttMgr();
    void begin();
    void maintain();
    void publish(const MeasurementData& m);
    void publishPresence(bool presence); // Ajout pour le temps réel
    void publishGasResistance(float resistance); // Ajout pour le temps réel
    bool isConnected();

private:
    WiFiClient   _espClient;
    PubSubClient _client;
    unsigned long _lastAttemptMs = 0;

    bool _connect();
    void _pubFloat(const char* topic, float value, uint8_t decimals = 1);
    void _pubInt  (const char* topic, int value);
    void _pubStr  (const char* topic, const char* value);
    void _pubULong(const char* topic, unsigned long value);
};
