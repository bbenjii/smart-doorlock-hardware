#include "services/ConfigStore.h"

ConfigStore::ConfigStore(NvsDriver& nvs) : _nvs(nvs) {}

bool ConfigStore::begin() {
    return _nvs.begin(NS, false);
}

void ConfigStore::end() {
    _nvs.end();
}

bool ConfigStore::loadAll(ConfigSnapshot& out) {
    bool ok = true;

    bool prov = false;
    ok &= getProvisioned(false, prov);
    out.isProvisioned = prov;

    bool hasCreds = false;
    WiFiCredentials creds;
    ok &= loadWiFi(creds, hasCreds);
    if (hasCreds) out.wifi = creds;

    String url;
    ok &= getBackendBaseUrl("", url);
    out.backendBaseUrl = url;

    return ok;
}

bool ConfigStore::setProvisioned(bool provisioned) {
    return _nvs.putBool(K_PROV, provisioned);
}

bool ConfigStore::getProvisioned(bool defaultValue, bool& out) {
    return _nvs.getBool(K_PROV, defaultValue, out);
}

bool ConfigStore::saveWiFi(const String& ssid, const String& password) {
    if (ssid.length() == 0) return false;
    bool ok = true;
    ok &= _nvs.putString(K_SSID, ssid);
    ok &= _nvs.putString(K_PASS, password);
    return ok;
}

bool ConfigStore::loadWiFi(WiFiCredentials& out, bool& hasCreds) {
    String ssid, pass;
    _nvs.getString(K_SSID, "", ssid);
    _nvs.getString(K_PASS, "", pass);

    hasCreds = ssid.length() > 0;
    if (hasCreds) {
        out.ssid = ssid;
        out.password = pass;
    }
    return true;
}

bool ConfigStore::clearWiFi() {
    bool ok = true;
    ok &= _nvs.remove(K_SSID);
    ok &= _nvs.remove(K_PASS);
    return ok;
}

bool ConfigStore::setBackendBaseUrl(const String& url) {
    if (url.length() == 0) return false;
    return _nvs.putString(K_BACKEND, url);
}

bool ConfigStore::getBackendBaseUrl(const String& defaultUrl, String& out) {
    return _nvs.getString(K_BACKEND, defaultUrl, out);
}

bool ConfigStore::factoryReset() {
    return _nvs.clear();
}

String ConfigStore::getDeviceId() const {
    // ESP32: derive from eFuse MAC similarly to your snippet
    uint64_t chipid = ESP.getEfuseMac();
    uint16_t chip = (uint16_t)(chipid >> 32);

    char devID[20];
    snprintf(devID, sizeof(devID), "%04X%08X", chip, (uint32_t)chipid);

    String s = String(devID);
    s.replace(":", "");

    return "smartlock_" + s;
}
