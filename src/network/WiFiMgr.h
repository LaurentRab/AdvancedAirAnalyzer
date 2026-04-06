#pragma once
#include <WiFi.h>

// ─── Gestionnaire WiFi ───────────────────────────────────
// connect()  : connexion bloquante au boot (timeout WIFI_TIMEOUT_MS)
// maintain() : à appeler dans loop() — reconnecte si déconnecté
class WiFiMgr {
public:
    void connect();
    void maintain();
    bool isConnected() const;
    int  rssi() const;

private:
    void          _doConnect();
    unsigned long _lastAttemptMs = 0;
};
