
#include <Arduino.h>
#include "services/BluetoothService.h"
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <ArduinoJson.h>



// UUIDs must match what the app is using
#define SERVICE_UUID "12345678-1234-1234-1234-1234567890ab"
#define COMMAND_CHAR_UUID "12345678-1234-1234-1234-1234567890ac"  // app writes here
#define STATE_CHAR_UUID   "12345678-1234-1234-1234-1234567890ad"  // app reads / subscribes


BluetoothService::BluetoothService(ConfigStore& configStore): _configStore(configStore) {
    // Constructor implementation (if needed)
}

// ==================== Characteristic callbacks: reacts to writes ====================

class CommandCallbacks : public BLECharacteristicCallbacks {

  public:
    CommandCallbacks(ConfigStore& config)
    : _config(config) {}

    void onWrite(BLECharacteristic* pCharacteristic) override {
      std::string rawValue = pCharacteristic->getValue();
      
      if (rawValue.empty()) {
        Serial.println("Empty BLE write");
        return;
      }

      String payload = String(rawValue.c_str());
      payload.trim();
      
      if (payload.length() == 0) {
        Serial.println("Command characteristic written with empty value");
        return;
      }

      // Convert to Arduino String for easy comparison with "LOCK" / "UNLOCK"
      String cmd = String(payload.c_str());
      cmd.trim();  // remove spaces and newlines

      // Try JSON first
      StaticJsonDocument<256> doc;
      DeserializationError err = deserializeJson(doc, payload);

      if (!err) {
        bool handled = false;

        if (doc.containsKey("ssid")) {
          const char* ssid = doc["ssid"];
          const char* pass = doc["password"] | "";

          bool ok = _config.saveWiFi(ssid, pass);
          Serial.printf("WiFi creds received: ssid='%s'\n", ssid);
          Serial.printf("WiFi creds save: %s\n", ok ? "OK" : "FAILED");
          handled = true;
        }

        if (doc.containsKey("backendBaseUrl")) {
          const char* url = doc["backendBaseUrl"];
          bool ok = _config.setBackendBaseUrl(url);
          Serial.printf("Backend URL received: %s\n", url);
          Serial.printf("Backend URL save: %s\n", ok ? "OK" : "FAILED");
          handled = true;
        }

        if (handled) {
          return;
        }
      }

      Serial.print("Received command: ");
      Serial.println(cmd);

      if (cmd.equalsIgnoreCase("LOCK")) {
        Serial.println("Lock command received");
      } else if (cmd.equalsIgnoreCase("UNLOCK")) {
        Serial.println("Unlock command received");
      } else {
        Serial.println("Unknown command");
      }
    }
  
    private:
        ConfigStore& _config;
};

void BluetoothService::begin() {
    // Initialize Bluetooth functionality here
    Serial.println("BluetoothService initialized");

    _deviceId = _configStore.getDeviceId();

      BLEDevice::init(_deviceId.c_str());
      
      _server = BLEDevice::createServer();

      _service = _server->createService(SERVICE_UUID);
    
      // Create command characteristic (WRITE from app)
    _commandChar = _service->createCharacteristic(
    COMMAND_CHAR_UUID,
    BLECharacteristic::PROPERTY_WRITE
  );

  _commandChar->setCallbacks(new CommandCallbacks(_configStore));

  // Create state characteristic (READ + NOTIFY to app)
  _stateChar = _service->createCharacteristic(
    STATE_CHAR_UUID,
    BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY
  );
}   

void BluetoothService::activateBluetooth() {
    if (_service) {
        _service->start();
        _server->getAdvertising()->start();
        Serial.println("Bluetooth activated and advertising started");
    } else {
        Serial.println("Error: Service not initialized");
    }
}

void BluetoothService::deactivateBluetooth() {
    if (_server) {
        _server->getAdvertising()->stop();
        Serial.println("Bluetooth deactivated and advertising stopped");
    } else {
        Serial.println("Error: Server not initialized");
    }
}
