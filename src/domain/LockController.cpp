#include "domain/LockController.h"

LockController::LockController(LedDriver& statusLed, DeviceStateStore& stateStore)
    : _statusLed(statusLed), _stateStore(stateStore) {}

void LockController::lock() {
    _isLocked = true;
    _statusLed.on();
    _stateStore.setLocked(true);
}

void LockController::unlock() {
    _isLocked = false;
    _statusLed.off();
    _stateStore.setLocked(false);
}

bool LockController::isLocked() const {
    return _isLocked;
}
