#include "services/DeviceStateStore.h"

DeviceStateStore::DeviceStateStore(NvsDriver& nvs) : _nvs(nvs) {}

bool DeviceStateStore::begin() {
    return _nvs.begin(NS, false);
}

void DeviceStateStore::end() {
    _nvs.end();
}

bool DeviceStateStore::load(DeviceStateSnapshot& out) {
    _nvs.getBool(K_LOCKED, true, out.isLocked);
    _nvs.getU32(K_BOOT, 0, out.bootCount);
    _nvs.getU32(K_LKC, 0, out.lockCount);
    _nvs.getU32(K_ULKC, 0, out.unlockCount);
    _nvs.getI32(K_ERR, 0, out.lastErrorCode);
    _nvs.getU32(K_ERRMS, 0, out.lastErrorAtMs);
    return true;
}

bool DeviceStateStore::incrementBootCount(uint32_t& outNewValue) {
    uint32_t v = 0;
    _nvs.getU32(K_BOOT, 0, v);
    v += 1;
    if (!_nvs.putU32(K_BOOT, v)) return false;
    outNewValue = v;
    return true;
}

bool DeviceStateStore::setLocked(bool isLocked) {
    return _nvs.putBool(K_LOCKED, isLocked);
}

bool DeviceStateStore::getLocked(bool defaultValue, bool& out) {
    return _nvs.getBool(K_LOCKED, defaultValue, out);
}

bool DeviceStateStore::incrementLockCount(uint32_t& outNewValue) {
    uint32_t v = 0;
    _nvs.getU32(K_LKC, 0, v);
    v += 1;
    if (!_nvs.putU32(K_LKC, v)) return false;
    outNewValue = v;
    return true;
}

bool DeviceStateStore::incrementUnlockCount(uint32_t& outNewValue) {
    uint32_t v = 0;
    _nvs.getU32(K_ULKC, 0, v);
    v += 1;
    if (!_nvs.putU32(K_ULKC, v)) return false;
    outNewValue = v;
    return true;
}

bool DeviceStateStore::setLastError(int32_t code, uint32_t atMs) {
    bool ok = true;
    ok &= _nvs.putI32(K_ERR, code);
    ok &= _nvs.putU32(K_ERRMS, atMs);
    return ok;
}

bool DeviceStateStore::getLastError(int32_t& outCode, uint32_t& outAtMs) {
    _nvs.getI32(K_ERR, 0, outCode);
    _nvs.getU32(K_ERRMS, 0, outAtMs);
    return true;
}

bool DeviceStateStore::clearAll() {
    return _nvs.clear();
}
