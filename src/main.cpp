#include <Arduino.h>
#include <WiFi.h>
#include <ArduinoWebsockets.h>
#include "esp_system.h"
#include "drivers/LedDriver.h"
#include "domain/LockController.h"
#include "services/WiFiService.h"
#include "services/WebSocketService.h"
#include "drivers/NvsDriver.h"
#include "services/ConfigStore.h"
#include "services/DeviceStateStore.h"
#include "drivers/ButtonDriver.h"
#include "services/BluetoothService.h"
#include "drivers/CameraDriver.h"
using namespace websockets;

// ===================== Setup / Loop ==========================
#define STATUS_PIN 13
#define STATE_PIN 14
#define ACTION_BUTTON_PIN 25
#define WEBSOCKET_BASE_URL "ws://192.168.2.208:8000/"

NvsDriver nvsCfg;
NvsDriver nvsState;

ConfigStore configStore(nvsCfg);
DeviceStateStore stateStore(nvsState);


WiFiService wifi(configStore);
LedDriver statusLed(STATUS_PIN);
LockController lockController(statusLed, stateStore);
WebSocketService ws(configStore, lockController, WEBSOCKET_BASE_URL);


// BluetoothService
BluetoothService bluetooth(configStore);

// Main Button Driver
ButtonDriver mainButton(ACTION_BUTTON_PIN); // GPIO pin 12 for the button

// State Led Driver 
LedDriver stateLed(STATE_PIN);


// Camera Driver 
CameraDriver camera;
bool cameraStarted = false;


enum class DeviceMode {
  Wifi,
  Bluetooth
};

DeviceMode currentMode = DeviceMode::Wifi;
unsigned long lastBlinkMs = 0;
bool blinkOn = false;
const unsigned long blinkIntervalMs = 500;

WiFiCredentials defaultWifiCreds = {
  "BEN", // Default SSID
  "Ben.1234" // Default Password
};

void applyMode(DeviceMode mode, bool force = false) {
  if (!force && mode == currentMode) return;

  currentMode = mode;

  if (currentMode == DeviceMode::Wifi) {
    bluetooth.deactivateBluetooth();
    wifi.begin();
    wifi.connect(20000);
    ws.begin();
    ws.connect();
    if (!cameraStarted) {
      camera.begin();
      cameraStarted = true;
    }

    stateLed.on();
    blinkOn = false;
    lastBlinkMs = millis();
    Serial.println("Mode set to WIFI");
    return;
  }

  ws.disconnect();
  wifi.disconnect();
  bluetooth.activateBluetooth();

  blinkOn = false;
  lastBlinkMs = millis();
  stateLed.off();
  Serial.println("Mode set to BLUETOOTH");
}

void smartLockSetUp() {
  // Placeholder for any additional setup logic
  statusLed.begin();
  
  stateLed.begin();
  
  mainButton.begin();

  configStore.begin();
  stateStore.begin();

  uint32_t boots = 0;
  stateStore.incrementBootCount(boots);
  Serial.printf("Boot count: %lu\n", (unsigned long)boots);

  // Restore lock state from persistent storage
  bool isLocked = true;
  stateStore.getLocked(true, isLocked);
  if (isLocked) lockController.lock();
  else lockController.unlock();

  // Load and display configuration
  ConfigSnapshot cfg;
  configStore.loadAll(cfg);

  if (cfg.isProvisioned) {
        Serial.println("Device provisioned");
        Serial.print("Backend URL: ");
        Serial.println(cfg.backendBaseUrl);
        Serial.print("WiFi SSID: ");
        Serial.println(cfg.wifi.ssid);
  } else {
        Serial.println("Device not provisioned");
  }

  bluetooth.begin();
  applyMode(DeviceMode::Wifi, true);

}

void setup() {
  Serial.begin(115200);
  smartLockSetUp();

}

void loop() {

  bool longPressDetected = mainButton.detect();
  // if (longPressDetected) {
  //   DeviceMode nextMode =
  //     (currentMode == DeviceMode::Wifi) ? DeviceMode::Bluetooth : DeviceMode::Wifi;
  //   applyMode(nextMode);
  // }

  // if (currentMode == DeviceMode::Bluetooth) {
  //   unsigned long now = millis();
  //   if (now - lastBlinkMs >= blinkIntervalMs) {
  //     lastBlinkMs = now;
  //     blinkOn = !blinkOn;
  //     if (blinkOn) stateLed.on();
  //     else stateLed.off();
  //   }
  // }
  ws.tick();
  // if (currentMode == DeviceMode::Wifi) {
  //   ws.tick();
  // }
  // Serial.println("Loop tick");
  delay(20);

}
