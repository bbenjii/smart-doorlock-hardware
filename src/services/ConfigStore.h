#pragma once
#include <Arduino.h>
#include "drivers/NvsDriver.h"

struct WiFiCredentials {
    String ssid;
    String password;
};

struct ConfigSnapshot {
    bool isProvisioned = false;
    WiFiCredentials wifi;
    String backendBaseUrl;  // example: "ws://192.168.2.208:8000/"
};

class ConfigStore {
public:
    explicit ConfigStore(NvsDriver& nvs);

    bool begin();
    void end();

    // High-level operations
    bool loadAll(ConfigSnapshot& out);

    bool setProvisioned(bool provisioned);
    bool getProvisioned(bool defaultValue, bool& out);

    bool saveWiFi(const String& ssid, const String& password);
    bool loadWiFi(WiFiCredentials& out, bool& hasCreds);
    bool clearWiFi();

    bool setBackendBaseUrl(const String& url);
    bool getBackendBaseUrl(const String& defaultUrl, String& out);

        String getDeviceId() const;

    // Optional: reset entire config namespace
    bool factoryReset();

private:
    NvsDriver& _nvs;

    static constexpr const char* NS = "cfg";
    static constexpr const char* K_PROV = "prov";
    static constexpr const char* K_SSID = "ssid";
    static constexpr const char* K_PASS = "pass";
    static constexpr const char* K_BACKEND = "burl";
};
