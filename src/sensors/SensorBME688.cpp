#include "SensorBME688.h"

// Initialisation du membre statique
SensorBME688::Data SensorBME688::_lastData;

bool SensorBME688::begin(uint8_t addr) {
    // bsec2.h définit begin(uint8_t i2cAddr, TwoWire &i2c, ...)
    if (!_iaqSensor.begin(addr, Wire)) {
        return false;
    }

    // Utilise les types bsecSensor (alias de bsec_virtual_sensor_t)
    bsecSensor sensorList[] = {
        BSEC_OUTPUT_IAQ,
        BSEC_OUTPUT_CO2_EQUIVALENT,
        BSEC_OUTPUT_BREATH_VOC_EQUIVALENT,
        BSEC_OUTPUT_RAW_PRESSURE,
        BSEC_OUTPUT_RAW_GAS,
        BSEC_OUTPUT_SENSOR_HEAT_COMPENSATED_TEMPERATURE,
        BSEC_OUTPUT_SENSOR_HEAT_COMPENSATED_HUMIDITY
    };

    // updateSubscription est défini dans bsec2.cpp
    if (!_iaqSensor.updateSubscription(sensorList, 7, BSEC_SAMPLE_RATE_ULP)) {
        return false;
    }

    // attachCallback attend un bsecCallback
    _iaqSensor.attachCallback(SensorBME688::bsecCallback);

    _ok = true;
    return true;
}

bool SensorBME688::loadState() {
    Preferences prefs;
    prefs.begin(NVS_NAMESPACE, /*readOnly=*/true);
    uint8_t state[BSEC_MAX_STATE_BLOB_SIZE];
    const size_t len = prefs.getBytes(NVS_KEY, state, sizeof(state));
    prefs.end();

    if (len == 0) {
        Serial.println(F("BSEC : aucun etat sauvegarde, calibration a froid."));
        return false;
    }
    if (!_iaqSensor.setState(state)) {
        Serial.println(F("BSEC : echec restauration etat NVS."));
        return false;
    }
    Serial.println(F("BSEC : etat restaure depuis NVS."));
    return true;
}

void SensorBME688::saveStateIfReady() {
    if (_lastData.iaqAccuracy < 3) return;

    const unsigned long now = millis();
    if (now - _lastSaveMs < SAVE_INTERVAL_MS) return;

    uint8_t state[BSEC_MAX_STATE_BLOB_SIZE];
    if (!_iaqSensor.getState(state)) {
        Serial.println(F("BSEC : echec lecture etat pour sauvegarde."));
        return;
    }

    Preferences prefs;
    prefs.begin(NVS_NAMESPACE, /*readOnly=*/false);
    prefs.putBytes(NVS_KEY, state, sizeof(state));
    prefs.end();

    _lastSaveMs = now;
    Serial.println(F("BSEC : etat sauvegarde en NVS."));
}

void SensorBME688::update() {
    if (!_ok) return;
    _iaqSensor.run(); // run() traite les données et appelle le callback
    saveStateIfReady();
}

void SensorBME688::bsecCallback(const bme68xData data, const bsecOutputs outputs, const Bsec2 bsec) {
    if (!outputs.nOutputs) return;

    _lastData.valid = true;
    for (uint8_t i = 0; i < outputs.nOutputs; i++) {
        const bsecData& out = outputs.output[i];
        switch (out.sensor_id) {
            case BSEC_OUTPUT_IAQ:
                _lastData.iaq = out.signal;
                _lastData.iaqAccuracy = out.accuracy;
                break;
            case BSEC_OUTPUT_RAW_PRESSURE:
                _lastData.pressure = out.signal; // Le bsec2.cpp convertit déjà Pa en hPa
                break;
            case BSEC_OUTPUT_RAW_GAS:
                _lastData.gasResistance = out.signal;
                break;
            case BSEC_OUTPUT_SENSOR_HEAT_COMPENSATED_TEMPERATURE:
                _lastData.temperature = out.signal;
                break;
            case BSEC_OUTPUT_SENSOR_HEAT_COMPENSATED_HUMIDITY:
                _lastData.humidity = out.signal;
                break;
            case BSEC_OUTPUT_CO2_EQUIVALENT:
                _lastData.co2Equiv = out.signal;
                break;
            case BSEC_OUTPUT_BREATH_VOC_EQUIVALENT:
                _lastData.breathVoc = out.signal;
                break;
        }
    }
}