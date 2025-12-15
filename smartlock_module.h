#pragma once

#include <WiFi.h>
#include <ArduinoWebsockets.h>
#include "esp_system.h"
#include "esp_camera.h"     // Already included in your main sketch, but safe to include here too

using namespace websockets;

// ===================== Wi-Fi / WS Config =====================


// Local dev server
#define WEBSOCKET_BASE_URL "ws://192.168.2.208:8000/"

// Device ID (derived from MAC)
String SMARTLOCK_DEVICE_ID = "";

// WebSocket client
WebsocketsClient smartlockClient;

// Connection state
bool smartlockWsConnected = false;
String smartlockWebsocketUrl;
unsigned long smartlockLastReconnectAttempt = 0;
const unsigned long SMARTLOCK_RECONNECT_INTERVAL_MS = 5000;

// ===================== Lock State / LED ======================

bool smartlockIsLocked = true;             // start locked
const int SMARTLOCK_LED_PIN = 2;

unsigned long smartlockLastStatusPost = 0;
const unsigned long SMARTLOCK_STATUS_INTERVAL = 10000;  // send status every 10s

inline void smartlockLightOn()  { digitalWrite(SMARTLOCK_LED_PIN, HIGH); }
inline void smartlockLightOff() { digitalWrite(SMARTLOCK_LED_PIN, LOW); }

// ===================== Helpers: MAC -> deviceId = smartlock_xxx ==================

String smartlockGetChipIdString() {
  uint64_t chipid = ESP.getEfuseMac();
  uint16_t chip = (uint16_t)(chipid >> 32);
  char devID[20];
  snprintf(devID, sizeof(devID), "%04X%08X", chip, (uint32_t)chipid);
  return String(devID);
}

// ===================== WS Helpers: Send JSON =================

void smartlockSendCommandFinished(const char* newStatus) {
  if (!smartlockWsConnected) return;

  String msg = "{";
  msg += "\"type\":\"command_finished\",";
  msg += "\"deviceId\":\"";
  msg += SMARTLOCK_DEVICE_ID;
  msg += "\",";
  msg += "\"new_status\":\"";
  msg += newStatus;
  msg += "\"}";

//  Serial.print("Sending command_finished: ");
//  Serial.println(msg);
  smartlockClient.send(msg);
}

void smartlockSendStatus() {
  if (!smartlockWsConnected) return;

  String statusStr = smartlockIsLocked ? "LOCKED" : "UNLOCKED";

  String msg = "{";
  msg += "\"type\":\"status\",";
  msg += "\"deviceId\":\"";
  msg += SMARTLOCK_DEVICE_ID;
  msg += "\",";
  msg += "\"status\":\"";
  msg += statusStr;
  msg += "\"}";

//  Serial.print("Sending status: ");
//  Serial.println(msg);
  smartlockClient.send(msg);
}

// ===================== Lock Logic ============================

void smartlockLockDoor() {
  Serial.println("LOCKING the door...");
  smartlockIsLocked = true;
  smartlockLightOn();

  // Notify server that command has finished
  smartlockSendCommandFinished("LOCKED");

  // Immediately push new status
  smartlockSendStatus();
}

void smartlockUnlockDoor() {
  Serial.println("UNLOCKING the door...");
  smartlockIsLocked = false;
  smartlockLightOff();

  // Notify server that command has finished
  smartlockSendCommandFinished("UNLOCKED");

  // Immediately push new status
  smartlockSendStatus();
}

// ===================== WS Callbacks ==========================

void smartlockOnMessageCallback(WebsocketsMessage message) {
  String msg = message.data();
  Serial.print("Received message: ");
  Serial.println(msg);

  if (msg == "LOCK") {
    smartlockLockDoor();
  } else if (msg == "UNLOCK") {
    smartlockUnlockDoor();
  } else if (msg == "get_status" || msg == "GET_STATUS") {
    Serial.println("Status requested via WS");
    smartlockSendStatus();
  } else {
    Serial.println("Unknown message received");
  }
}

void smartlockOnEventsCallback(WebsocketsEvent event, String data) {
  if (event == WebsocketsEvent::ConnectionOpened) {
    Serial.println("WebSocket Connected");
    smartlockWsConnected = true;
  } else if (event == WebsocketsEvent::ConnectionClosed) {
    Serial.println("WebSocket Closed");
    smartlockWsConnected = false;
  }
}

// ===================== WS Connect / Reconnect ================

void smartlockConnectWebSocket() {
  Serial.print("Connecting to WebSocket URL: ");
  Serial.println(smartlockWebsocketUrl);

  smartlockClient.setInsecure(); // harmless for ws://; required if you later switch to wss

  bool ok = smartlockClient.connect(smartlockWebsocketUrl.c_str());
  if (ok) {
    Serial.println("WebSocket connection established");
    smartlockWsConnected = true;

    String hello = "{";
    hello += "\"type\":\"hello\",";
    hello += "\"deviceId\":\"";
    hello += SMARTLOCK_DEVICE_ID;
    hello += "\"}";
    smartlockClient.send(hello);
  } else {
    Serial.println("WebSocket connect failed");
    smartlockWsConnected = false;
  }
}

// ===================== Public API for main sketch ============

// Call this from your global setup()
void SmartLock_setup() {
  // Build DEVICE_ID from MAC
  String mac = smartlockGetChipIdString();
  Serial.print("Raw chip string: ");
  Serial.println(mac);

  mac.replace(":", "");
  SMARTLOCK_DEVICE_ID = "smartlock_" + mac;

  Serial.print("Device ID: ");
  Serial.println(SMARTLOCK_DEVICE_ID);

  delay(200);

  pinMode(SMARTLOCK_LED_PIN, OUTPUT);
  if (smartlockIsLocked) smartlockLightOn();
  else smartlockLightOff();



  smartlockClient.onMessage(smartlockOnMessageCallback);
  smartlockClient.onEvent(smartlockOnEventsCallback);

  smartlockWebsocketUrl = String(WEBSOCKET_BASE_URL) + "ws/device";

  // Initial connect
  while (!smartlockWsConnected) {
    smartlockConnectWebSocket();
    if (!smartlockWsConnected) {
      Serial.println("Retrying WS connection in 2 seconds...");
      delay(2000);
    }
  }

  smartlockLastReconnectAttempt = millis();
  smartlockLastStatusPost = millis();
}


void smartlockSendCameraFrame() {
  if (!smartlockWsConnected) {
    return;
  }

  camera_fb_t *fb = esp_camera_fb_get();
  if (!fb) {
    Serial.println("Camera capture failed");
    return;
  }

  // Send JPEG frame as a binary WebSocket message
  smartlockClient.sendBinary((const char*)fb->buf, fb->len);

  esp_camera_fb_return(fb);
}



unsigned long lastFrameSent = 0;
const unsigned long FRAME_INTERVAL_MS = 50;  // every 1 second

// Call this from your global loop()
void SmartLock_loop() {
  // LED reflects lock state
  if (smartlockIsLocked) smartlockLightOn();
  else smartlockLightOff();

  if (smartlockWsConnected) {
    smartlockClient.poll();
  } else {
    unsigned long now = millis();
    if (now - smartlockLastReconnectAttempt > SMARTLOCK_RECONNECT_INTERVAL_MS) {
      Serial.println("Attempting WebSocket reconnection...");
      smartlockLastReconnectAttempt = now;
      smartlockConnectWebSocket();
    }
  }

  // Periodic status push
  if (smartlockWsConnected && (millis() - smartlockLastStatusPost >= SMARTLOCK_STATUS_INTERVAL)) {
    smartlockLastStatusPost = millis();
    smartlockSendStatus();
  }


  unsigned long now = millis();
  if (now - lastFrameSent >= FRAME_INTERVAL_MS) {
    lastFrameSent = now;
    smartlockSendCameraFrame();
  }

  delay(50);
}
