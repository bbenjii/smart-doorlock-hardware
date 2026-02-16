#include "drivers/NvsDriver.h"

NvsDriver::NvsDriver() {}

bool NvsDriver::begin(const char* nameSpace, bool readOnly) {
    _begun = _prefs.begin(nameSpace, readOnly);
    return _begun;
}

void NvsDriver::end() {
    if (_begun) {
        _prefs.end();
        _begun = false;
    }
}

bool NvsDriver::putBool(const char* key, bool value) {
    if (!_begun) return false;
    return _prefs.putBool(key, value) > 0;
}

bool NvsDriver::getBool(const char* key, bool defaultValue, bool& outValue) const {
    if (!_begun) { outValue = defaultValue; return false; }
    outValue = _prefs.getBool(key, defaultValue);
    return true;
}

bool NvsDriver::putU32(const char* key, uint32_t value) {
    if (!_begun) return false;
    return _prefs.putUInt(key, value) > 0;
}

bool NvsDriver::getU32(const char* key, uint32_t defaultValue, uint32_t& outValue) const {
    if (!_begun) { outValue = defaultValue; return false; }
    outValue = _prefs.getUInt(key, defaultValue);
    return true;
}

bool NvsDriver::putI32(const char* key, int32_t value) {
    if (!_begun) return false;
    return _prefs.putInt(key, value) > 0;
}

bool NvsDriver::getI32(const char* key, int32_t defaultValue, int32_t& outValue) const {
    if (!_begun) { outValue = defaultValue; return false; }
    outValue = _prefs.getInt(key, defaultValue);
    return true;
}

bool NvsDriver::putString(const char* key, const String& value) {
    if (!_begun) return false;
    return _prefs.putString(key, value) > 0;
}

bool NvsDriver::getString(const char* key, const String& defaultValue, String& outValue) const {
    if (!_begun) { outValue = defaultValue; return false; }
    outValue = _prefs.getString(key, defaultValue);
    return true;
}

bool NvsDriver::remove(const char* key) {
    if (!_begun) return false;
    return _prefs.remove(key);
}

bool NvsDriver::clear() {
    if (!_begun) return false;
    return _prefs.clear();
}
