#pragma once
#include "secrets.h"

// ─── Identité appareil ────────────────────────────────────
#define DEVICE_NAME   "AdvancedAirAnalyzer"
#define FW_VERSION    "1.0.0"

// ─── Broches ESP32-C3 Mini ────────────────────────────────
#define I2C_SDA         8
#define I2C_SCL         9
#define PIN_RADAR       10    // LD2410C : sortie logique OUT (présence)
#define PMS_RX          20
#define PMS_TX          21
#define PIN_PMS_SET     5     // PMS5003 : SET LOW = veille hardware (ventilateur arrêté)

// ─── Timings ──────────────────────────────────────────────
#define MEASURE_INTERVAL_MS   (5UL * 60UL * 1000UL)   // 5 min
#define PMS_WARMUP_MS         30000UL                 // 30 s stabilisation ventilateur

// Offset d'auto-échauffement du SCD40 (°C).
// Le SCD40 lit plus chaud que l'ambiant réel à cause de son électronique.
// Ajuster empiriquement : delta = temp_SCD - temp_BME en conditions stables.
// Sensirion recommande 4 °C pour un boîtier fermé, ~2 °C en espace ouvert.
// L'offset améliore la précision du calcul CO2 interne du SCD40.
#define SCD40_TEMP_OFFSET     4.0f   // °C — à affiner avec vos mesures
#define WIFI_TIMEOUT_MS       30000UL
#define MQTT_RECONNECT_MS     5000UL
#define WDT_TIMEOUT_S         120                        // 2 min

// ─── Topics MQTT ─────────────────────────────────────────
#define MQTT_BASE             "home/Advanced_air_analyzer"

// Système
#define MQTT_TOPIC_STATUS     MQTT_BASE "/status"
#define MQTT_TOPIC_RSSI       MQTT_BASE "/rssi"
#define MQTT_TOPIC_UPTIME     MQTT_BASE "/uptime"

// SCD40
#define MQTT_TOPIC_CO2        MQTT_BASE "/co2"
// T/H du SCD40 publiées à titre de diagnostic uniquement :
// le SCD40 s'auto-échauffe (+2 à +5 °C), ses valeurs T/H ne reflètent
// pas l'ambiant réel mais permettent de calculer l'offset de compensation.
#define MQTT_TOPIC_TEMP_SCD   MQTT_BASE "/temperature_scd"
#define MQTT_TOPIC_HUM_SCD    MQTT_BASE "/humidity_scd"

// BME688 — T/H de référence (pas d'auto-échauffement continu)
#define MQTT_TOPIC_TEMP       MQTT_BASE "/temperature"    // primaire
#define MQTT_TOPIC_HUM        MQTT_BASE "/humidity"       // primaire
#define MQTT_TOPIC_PRESSURE   MQTT_BASE "/pressure"
#define MQTT_TOPIC_GAS        MQTT_BASE "/gas_resistance_kohm"
#define MQTT_TOPIC_IAQ        MQTT_BASE "/iaq"
#define MQTT_TOPIC_IAQ_ACCURACY  MQTT_BASE "/iaq_accuracy"
#define MQTT_TOPIC_ECO2       MQTT_BASE "/eco2"

// PMS5003 — concentration massique atmosphérique (AE, terrain extérieur/intérieur)
#define MQTT_TOPIC_PM1_AE     MQTT_BASE "/pm1_ae"
#define MQTT_TOPIC_PM25_AE    MQTT_BASE "/pm2_5_ae"
#define MQTT_TOPIC_PM10_AE    MQTT_BASE "/pm10_ae"
// PMS5003 — concentration massique standard CF=1 (référence laboratoire)
#define MQTT_TOPIC_PM1_SP     MQTT_BASE "/pm1_sp"
#define MQTT_TOPIC_PM25_SP    MQTT_BASE "/pm2_5_sp"
#define MQTT_TOPIC_PM10_SP    MQTT_BASE "/pm10_sp"

// LD2410C
#define MQTT_TOPIC_PRESENCE   MQTT_BASE "/presence"

#define MQTT_TOPIC_HEALTH_SCORE MQTT_BASE "/health_score"
