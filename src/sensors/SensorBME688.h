#pragma once
#include <Arduino.h>
#include "bsec2.h"

class SensorBME688 {
public:
    struct Data {
        float temperature = 0.0f;
        float humidity    = 0.0f;
        float pressure    = 0.0f;
        float iaq         = 0.0f;
        uint8_t iaqAccuracy = 0;
        float co2Equiv    = 0.0f;
        float breathVoc   = 0.0f;
        float gasResistance = 0.0f;
        bool  valid       = false;
    };

    bool begin(uint8_t addr = 0x76);
    void update();
    Data getData() { return _lastData; }
    float getGasResistance() { return _lastData.gasResistance; }

private:
    Bsec2 _iaqSensor;
    static Data _lastData; // Statique pour être accessible par le callback statique
    bool _ok = false;

    // Signature exacte correspondant à bsecCallback dans bsec2.h
    static void bsecCallback(const bme68xData data, const bsecOutputs outputs, const Bsec2 bsec);
};