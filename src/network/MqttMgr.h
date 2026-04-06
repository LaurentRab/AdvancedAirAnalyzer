#pragma once
#include <PubSubClient.h>
#include <WiFiClient.h>

// ─── Structure agrégée de toutes les mesures ─────────────────────────────
// Découple la couche réseau des modules capteurs.
struct MeasurementData {
    // SCD40 — CO2 uniquement (T/H publiées à titre de diagnostic)
    uint16_t co2      = 0;
    float    tempSCD  = 0.0f;   // °C — biaisé par auto-échauffement, usage diagnostic
    float    humSCD   = 0.0f;   // %RH — idem
    bool     scdValid = false;

    // BME688 — T/H de référence + pression + résistance gaz
    float temperature   = 0.0f;   // °C — primaire (pas d'auto-échauffement)
    float humidity      = 0.0f;   // %RH — primaire
    float pressure      = 0.0f;   // hPa
    float gasResistance = 0.0f;   // Ω (÷1000 pour kΩ)
    bool  bmeValid      = false;

    // PMS5003 — particules fines (µg/m³)
    // AE = Atmospheric Environment (valeurs terrain, pour IAQ)
    uint16_t pm1_ae  = 0;
    uint16_t pm25_ae = 0;
    uint16_t pm10_ae = 0;
    // SP = Standard Particles CF=1 (valeurs de référence, pour normes)
    uint16_t pm1_sp  = 0;
    uint16_t pm25_sp = 0;
    uint16_t pm10_sp = 0;
    bool     pmsValid = false;

    // LD2410C — présence (sortie logique OUT)
    bool presence = false;

    // Système
    int rssi = 0;
    unsigned long uptime = 0; // secondes
};

// ─── Gestionnaire MQTT ────────────────────────────────────────────────────
class MqttMgr {
public:
    MqttMgr();
    void begin();
    void maintain();
    void publish(const MeasurementData& m);
    void publishPresence(bool presence); // Ajout pour le temps réel
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
