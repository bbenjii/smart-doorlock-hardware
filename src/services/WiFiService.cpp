#include "services/WiFiService.h"

WiFiService::WiFiService(ConfigStore& configStore)
    : _configStore(configStore) {}

void WiFiService::begin() {
    // Load WiFi credentials from persistent storage
    WiFiCredentials creds;
    bool hasCreds = false;
    _configStore.loadWiFi(creds, hasCreds);
    
    if (hasCreds) {
        _ssid = creds.ssid;
        _password = creds.password;
    } else {
        Serial.println("Warning: No WiFi credentials stored");
        return;
    }
    
    WiFi.mode(WIFI_STA);
    WiFi.disconnect(true, true);
    delay(200);
}

void WiFiService::begin(WiFiCredentials& defaultCreds) {
    // Load WiFi credentials from parameters, fallback to persistent storage
    WiFiCredentials creds = defaultCreds;
    bool hasCreds = !defaultCreds.ssid.isEmpty();
    
    // If default credentials not available, try to load from persistent storage
    if (!hasCreds) {
        _configStore.loadWiFi(creds, hasCreds);
    }
    
    if (hasCreds) {
        _ssid = creds.ssid;
        _password = creds.password;
    } else {
        Serial.println("Warning: No WiFi credentials provided or stored");
        return;
    }
    
    WiFi.mode(WIFI_STA);
    WiFi.disconnect(true, true);
    delay(200);
}

bool WiFiService::connect(uint32_t timeoutMs) {
    if (_ssid.isEmpty() || _password.isEmpty()) {
        Serial.println("WiFi credentials not loaded");
        return false;
    }

    Serial.println("Connecting to WiFi...");
    Serial.println("Wifi: " + _ssid);
    Serial.println("Wifi: " + _password);
    WiFi.begin(_ssid, _password.c_str());

    unsigned long start = millis();
    wl_status_t lastStatus = WL_IDLE_STATUS;

    while (millis() - start < timeoutMs) {
        wl_status_t status = WiFi.status();

        if (status != lastStatus) {
            Serial.printf("WiFi status: %d\n", status);
            lastStatus = status;
        }

        if (status == WL_CONNECTED) {
            Serial.println("WiFi connected!");
            Serial.print("IP: ");
            Serial.println(WiFi.localIP());
            return true;
        }

        delay(500);
    }

    Serial.println("WiFi connection timeout");
    Serial.printf("Final status: %d\n", WiFi.status());
    Serial.printf("Final status: %d\n", WiFi.status());
    return false;
}

void WiFiService::disconnect() {
    WiFi.disconnect(true);
}

bool WiFiService::isConnected() const {
    return WiFi.status() == WL_CONNECTED;
}
