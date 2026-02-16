#pragma once
#include "drivers/LedDriver.h"
#include "services/DeviceStateStore.h"

class LockController {
public:
    explicit LockController(LedDriver& statusLed, DeviceStateStore& stateStore);

    void lock();
    void unlock();
    bool isLocked() const;

private:
    LedDriver& _statusLed;
    DeviceStateStore& _stateStore;
    bool _isLocked = true;
};
