#pragma once

// ─── Gestionnaire OTA (Over-The-Air) ─────────────────────
// Utilise ArduinoOTA (intégré à l'ESP32 Arduino framework).
// Upload via : pio run -e lolin_c3_mini_ota -t upload
// Mot de passe OTA : défini dans include/secrets.h (OTA_PASSWORD)
class OtaMgr {
public:
    void begin();
    void handle();   // à appeler dans chaque itération de loop()
};
