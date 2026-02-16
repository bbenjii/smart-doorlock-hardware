#pragma once
#include <Arduino.h>
#include <BLEServer.h>
#include "services/ConfigStore.h"

class BluetoothService {
    public:
        BluetoothService(ConfigStore& configStore);
        void begin();

        void activateBluetooth();
        void deactivateBluetooth();

    private:        
        String _deviceId;
        ConfigStore& _configStore;

        BLEServer* _server = nullptr;
        BLEService* _service = nullptr;

        BLECharacteristic* _commandChar = nullptr;
        BLECharacteristic* _stateChar   = nullptr;



};