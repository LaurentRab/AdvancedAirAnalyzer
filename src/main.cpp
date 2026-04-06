/**
 * AdvancedAirAnalyzer -- main.cpp
 *
 * Cycle de mesure toutes les 15 minutes :
 *   T-30 s  -> wakeUp PMS5003 (SET HIGH + warmup ventilateur)
 *   T+0     -> lecture SCD40 + BME688 + PMS5003 + LD2410C -> MQTT
 *   T+0     -> sleep PMS5003 (SET LOW : ventilateur arrete)
 *
 * OTA : pio run -e lolin_c3_mini_ota -t upload
 */

#include <Arduino.h>
#include <Wire.h>
#include <esp_task_wdt.h>

#include "config.h"
#include "sensors/SensorSCD40.h"
#include "sensors/SensorBME688.h"
#include "sensors/SensorPMS5003.h"
#include "sensors/SensorLD2410C.h"
#include "network/WiFiMgr.h"
#include "network/MqttMgr.h"
#include "ota/OtaMgr.h"

// --- Instances des modules ---
static SensorSCD40   sensorSCD40;
static SensorBME688  sensorBME688;
static SensorPMS5003 sensorPMS5003;
static SensorLD2410C sensorLD2410C;
static WiFiMgr       wifiMgr;
static MqttMgr       mqttMgr;
static OtaMgr        otaMgr;

// --- Etat du cycle de mesure ---
static unsigned long lastMeasureMs      = 0;
static unsigned long pmsWakeMs          = 0;
static bool          pmsAwake           = false;
static bool          lastPresenceState  = false;
static unsigned long lastPresenceCheckMs= 0;
static float         lastGasResistance  = 0.0f;

static void measureAndPublish();

// ---------------------------------------------------------
void setup() {
    Serial.begin(115200);
    delay(2000);
    Serial.println(F("\n=== " DEVICE_NAME " v" FW_VERSION " ===\n"));

    // Watchdog : redémarre si loop() bloque > WDT_TIMEOUT_S secondes
    esp_task_wdt_init(WDT_TIMEOUT_S, true);
    esp_task_wdt_add(NULL);

    // Capteurs I2C
    Wire.begin(I2C_SDA, I2C_SCL);
    sensorSCD40.begin();
    sensorBME688.begin();

    // PMS5003 : rxPin, txPin, setPin (GPIO 5 = controle hardware)
    sensorPMS5003.begin(PMS_RX, PMS_TX, PIN_PMS_SET);

    // LD2410C : sortie logique OUT
    sensorLD2410C.begin(PIN_RADAR);

    // Reseau + OTA
    wifiMgr.connect();
    otaMgr.begin();
    mqttMgr.begin();

    // Force la premiere mesure apres le warmup PMS (30 s)
    lastMeasureMs = millis() - MEASURE_INTERVAL_MS;
    Serial.printf("[Main] Pret -- premiere mesure dans %lu s\n",
                  PMS_WARMUP_MS / 1000UL);
}

// ---------------------------------------------------------
void loop() {
    esp_task_wdt_reset();

    // Crucial : BSEC2 doit "tourner" en permanence
    sensorBME688.update();

    wifiMgr.maintain();
    otaMgr.handle();
    mqttMgr.maintain();

    const unsigned long now = millis();

    // --- AJOUT : Gestion de la présence en temps réel et de la résistance du gaz --
    if (now - lastPresenceCheckMs >= 500) { // Check toutes les 500ms
        bool currentPresence = sensorLD2410C.read();
        float currentGasResistance = sensorBME688.getGasResistance();
        if (currentPresence != lastPresenceState) {
            // On publie immédiatement le changement d'état
            mqttMgr.publishPresence(currentPresence); 
            lastPresenceState = currentPresence;
        }
        if (currentGasResistance != lastGasResistance) {
            // On publie immédiatement le changement de résistance
            mqttMgr.publishGasResistance(currentGasResistance);
            lastGasResistance = currentGasResistance;
        }
        lastPresenceCheckMs = now;
    }
    // Reveil PMS5003 30 s avant la prochaine mesure
    if (!pmsAwake &&
        (now - lastMeasureMs >= MEASURE_INTERVAL_MS - PMS_WARMUP_MS)) {
        sensorPMS5003.wakeUp();
        pmsWakeMs = now;
        pmsAwake  = true;
    }

    // Mesure une fois le warmup ecoule
    if (pmsAwake && (now - pmsWakeMs >= PMS_WARMUP_MS)) {
        measureAndPublish();
        sensorPMS5003.sleep();
        pmsAwake      = false;
        lastMeasureMs = now;
    }
}

// ---------------------------------------------------------
static void measureAndPublish() {
    Serial.println(F("\n--- Mesure ---"));

    SensorSCD40::Data   scd = sensorSCD40.read();
    // Correction : on récupère la dernière donnée calculée par BSEC2
    SensorBME688::Data  bme = sensorBME688.getData();
    SensorPMS5003::Data pms = sensorPMS5003.read(3000); // timeout 3 s (mode passif : 1 requête → 1 trame ~200 ms, marge x15)
    const bool presence     = sensorLD2410C.read();
    const int  rssi         = wifiMgr.rssi();

    if (scd.valid)
        Serial.printf("  SCD40   CO2: %d ppm | Temp(SCD): %.1f C | Hum(SCD): %.1f %%\n",
                      scd.co2, scd.temperature, scd.humidity);
    else
        Serial.println(F("  SCD40   donnees invalides"));

    // Publication de l'IAQ réel et de sa précision    
    if (bme.valid)
        Serial.printf("  BME688  Temp: %.1f C | Hum: %.1f %% | Pres: %.1f hPa | Gaz: %.1f kohm\n",
                      bme.temperature, bme.humidity, bme.pressure, bme.gasResistance / 1000.0f);
    else
        Serial.println(F("  BME688  donnees invalides"));

    if (!pms.valid)
        Serial.println(F("  PMS5003 donnees invalides"));

    Serial.printf("  LD2410C Presence: %s | RSSI: %d dBm\n",
                  presence ? "OUI" : "NON", rssi);
    Serial.println(F("--------------"));

    // Aggregation pour publication MQTT
    MeasurementData m;
    m.co2     = scd.co2;
    m.tempSCD = scd.temperature;   // diagnostic auto-echauffement
    m.humSCD  = scd.humidity;
    m.scdValid = scd.valid;

    m.temperature   = bme.temperature;   // reference ambiant (primaire)
    m.humidity      = bme.humidity;
    m.pressure      = bme.pressure;
    m.gasResistance = bme.gasResistance;
    m.bmeValid      = bme.valid;
    m.iaq           = bme.iaq;
    m.iaqAccuracy   = bme.iaqAccuracy;
    m.co2Equiv      = bme.co2Equiv;
    m.breathVoc     = bme.breathVoc;
    m.healthScore   = 0.0f; // Initialisé à 0, sera calculé ci-dessous

    // --- CALCUL DE L'INDEX DE CONFINEMENT (Santé Globale) ---
    // On normalise les deux sources de pollution sur une échelle de 0 à 1
    float co2Penalty = (m.co2 - 400.0f) / 1600.0f; // 400ppm=0, 2000ppm=1
    float iaqPenalty = m.iaq / 500.0f;             // 0=0, 500=1
    
    // Le score final prend la pire des deux pénalités
    float maxPenalty = max(co2Penalty, iaqPenalty);
    if (maxPenalty < 0.0f) maxPenalty = 0.0f;
    if (maxPenalty > 1.0f) maxPenalty = 1.0f;

    m.healthScore = (1.0f - maxPenalty) * 100.0f; // Score 0-100%


    m.pm1_ae  = pms.pm1_ae;
    m.pm25_ae = pms.pm25_ae;
    m.pm10_ae = pms.pm10_ae;
    m.pm1_sp  = pms.pm1_sp;
    m.pm25_sp = pms.pm25_sp;
    m.pm10_sp = pms.pm10_sp;
    m.pmsValid = pms.valid;

    m.presence = presence;
    m.rssi     = rssi;
    m.uptime   = (millis() / 60000UL);
    mqttMgr.publish(m);
}
