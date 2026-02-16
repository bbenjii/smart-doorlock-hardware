#pragma once
#include <Arduino.h>
#include <Preferences.h>

class NvsDriver {
public:
    NvsDriver();

    // Open a namespace (NVS partition "nvs" by default in Arduino core)
    bool begin(const char* nameSpace, bool readOnly = false);
    void end();

    // Primitives
    bool putBool(const char* key, bool value);
    bool getBool(const char* key, bool defaultValue, bool& outValue) const;

    bool putU32(const char* key, uint32_t value);
    bool getU32(const char* key, uint32_t defaultValue, uint32_t& outValue) const;

    bool putI32(const char* key, int32_t value);
    bool getI32(const char* key, int32_t defaultValue, int32_t& outValue) const;

    bool putString(const char* key, const String& value);
    bool getString(const char* key, const String& defaultValue, String& outValue) const;

    bool remove(const char* key);
    bool clear();

private:
    mutable Preferences _prefs;
    bool _begun = false;
};
