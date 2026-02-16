#pragma once
#include <Arduino.h>
#include <WiFi.h>
#include "services/ConfigStore.h"

class WiFiService {
public:
    explicit WiFiService(ConfigStore& configStore);

    void begin();
    void begin(WiFiCredentials& defaultCreds);

    bool connect(uint32_t timeoutMs = 20000);
    void disconnect();

    bool isConnected() const;

private:
    ConfigStore& _configStore;
    String _ssid;
    String _password;
};
