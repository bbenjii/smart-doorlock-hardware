#pragma once
#include <Arduino.h>
#include "drivers/NvsDriver.h"

struct DeviceStateSnapshot {
    bool isLocked = true;

    uint32_t bootCount = 0;
    uint32_t lockCount = 0;
    uint32_t unlockCount = 0;

    int32_t lastErrorCode = 0;      // define your own error enum/codes
    uint32_t lastErrorAtMs = 0;     // optional, millis() at time of error
};

class DeviceStateStore {
public:
    explicit DeviceStateStore(NvsDriver& nvs);

    bool begin();
    void end();

    bool load(DeviceStateSnapshot& out);

    // Boot counter
    bool incrementBootCount(uint32_t& outNewValue);

    // Lock state
    bool setLocked(bool isLocked);
    bool getLocked(bool defaultValue, bool& out);

    // Counters
    bool incrementLockCount(uint32_t& outNewValue);
    bool incrementUnlockCount(uint32_t& outNewValue);

    // Error
    bool setLastError(int32_t code, uint32_t atMs);
    bool getLastError(int32_t& outCode, uint32_t& outAtMs);

    // Optional: clear state namespace
    bool clearAll();

private:
    NvsDriver& _nvs;

    static constexpr const char* NS = "state";
    static constexpr const char* K_LOCKED = "locked";
    static constexpr const char* K_BOOT = "boot";
    static constexpr const char* K_LKC = "lkc";
    static constexpr const char* K_ULKC = "ulkc";
    static constexpr const char* K_ERR = "err";
    static constexpr const char* K_ERRMS = "errms";
};
