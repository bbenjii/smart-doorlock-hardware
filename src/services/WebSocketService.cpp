#include "services/WebSocketService.h"
#include <WiFi.h>
#include "esp_system.h"
#include "esp_camera.h" 

using namespace websockets;

unsigned long lastFrameSent = 0;
const unsigned long FRAME_INTERVAL_MS = 50;  // every 1 second

WebSocketService::WebSocketService(ConfigStore& configStore,
                                   LockController& lockController,
                                   const char* defaultBaseUrl)
    : _configStore(configStore),
      _baseUrl(""),
      _defaultBaseUrl(defaultBaseUrl),
      _lockController(lockController) {}

    
void WebSocketService::begin() {
    _deviceId = buildDeviceId();

    refreshWsUrl_();

    _client.onMessage([this](WebsocketsMessage msg) { this->onMessage_(msg); });
    _client.onEvent([this](WebsocketsEvent ev, String data) { this->onEvent_(ev, data); });

    _lastReconnectAttempt = millis();
    _lastStatusPost = millis();
}

bool WebSocketService::connect(uint32_t timeoutMs) {
    if (WiFi.status() != WL_CONNECTED) {
        _connected = false;
        return false;
    }

    refreshWsUrl_();
    Serial.print("Connecting to WebSocket URL: ");
    Serial.println(_wsUrl);
    Serial.print("WiFi status: ");
    Serial.println(WiFi.status());

    _client.setInsecure(); // harmless for ws://, needed if you move to wss later

    unsigned long start = millis();
    while (millis() - start < timeoutMs) {
        bool ok = _client.connect(_wsUrl.c_str());
        if (ok) {
            _connected = true;

            Serial.println("WebSocket connection established");

            String hello = "{";
            hello += "\"type\":\"hello\",";
            hello += "\"deviceId\":\"";
            hello += _deviceId;
            hello += "\"}";
            _client.send(hello);

            return true;
        }

        delay(200);
    }

    Serial.println("WebSocket connect failed");
    _connected = false;
    return false;
}

void WebSocketService::disconnect() {
    _client.close();
    _connected = false;
}

bool WebSocketService::isConnected() const {
    return _connected;
}

const String& WebSocketService::deviceId() const {
    return _deviceId;
}

void WebSocketService::smartlockSendCameraFrame() {
  if (!_connected) {
    Serial.println("Not connected, cannot send camera frame");
    return;
  }
  if (!_client.available()) {
    Serial.println("WebSocket not available for camera frame");
    _connected = false;
    return;
  }
//   Serial.println("Capturing camera frame...");

  camera_fb_t *fb = esp_camera_fb_get();
  if (!fb) {
    Serial.println("Camera capture failed");
    return;
  }

  // Send JPEG frame as a binary WebSocket message
  _client.sendBinary((const char*)fb->buf, fb->len);
//   if (ok) {
//     Serial.println("Sent camera frame");
//   } else {
//     Serial.println("Failed to send camera frame");
//     _connected = false;
//     _client.close();
//   }

  esp_camera_fb_return(fb);
//   return ok;
return;
}

void WebSocketService::tick() {
    if (_connected) {
        _client.poll();

        if (millis() - _lastStatusPost >= _statusIntervalMs) {
            _lastStatusPost = millis();
            sendStatus();
        }

        unsigned long now = millis();
        if (now - lastFrameSent >= FRAME_INTERVAL_MS) {
            smartlockSendCameraFrame();
            lastFrameSent = now;
        }


        return;
    }

    tryReconnect_();
}

void WebSocketService::tryReconnect_() {
    unsigned long now = millis();
    if (now - _lastReconnectAttempt < _reconnectIntervalMs) return;

    _lastReconnectAttempt = now;
    _client.close();
    Serial.println("Attempting WebSocket reconnection...");
    connect();
}

String WebSocketService::buildDeviceId() const {
    // ESP32: derive from eFuse MAC similarly to your snippet
    uint64_t chipid = ESP.getEfuseMac();
    uint16_t chip = (uint16_t)(chipid >> 32);

    char devID[20];
    snprintf(devID, sizeof(devID), "%04X%08X", chip, (uint32_t)chipid);

    String s = String(devID);
    s.replace(":", "");

    return "smartlock_" + s;
}

String WebSocketService::buildWsUrl_(const String& baseUrl) const {
    if (baseUrl.endsWith("/")) {
        return baseUrl + "ws/device";
    }
    return baseUrl + "/ws/device";
}

void WebSocketService::refreshWsUrl_() {
    String url;
    _configStore.getBackendBaseUrl(_defaultBaseUrl, url);
    if (url.length() == 0) return;

    _baseUrl = url;
    _wsUrl = buildWsUrl_(_baseUrl);
}

void WebSocketService::sendCommandFinished(const char* newStatus) {
    if (!_connected) return;

    String msg = "{";
    msg += "\"type\":\"command_finished\",";
    msg += "\"deviceId\":\"";
    msg += _deviceId;
    msg += "\",";
    msg += "\"new_status\":\"";
    msg += newStatus;
    msg += "\"}";
    _client.send(msg);
}

void WebSocketService::sendStatus() {
    if (!_connected) return;

    const char* statusStr = _lockController.isLocked() ? "LOCKED" : "UNLOCKED";

    String msg = "{";
    msg += "\"type\":\"status\",";
    msg += "\"deviceId\":\"";
    msg += _deviceId;
    msg += "\",";
    msg += "\"status\":\"";
    msg += statusStr;
    msg += "\"}";
    _client.send(msg);
}

void WebSocketService::onMessage_(WebsocketsMessage message) {
    String msg = message.data();

    Serial.print("Received message: ");
    Serial.println(msg);

    if (msg == "LOCK") {
        _lockController.lock();
        sendCommandFinished("LOCKED");
        sendStatus();
        return;
    }

    if (msg == "UNLOCK") {
        _lockController.unlock();
        sendCommandFinished("UNLOCKED");
        sendStatus();
        return;
    }

    if (msg == "get_status" || msg == "GET_STATUS") {
        Serial.println("Status requested via WS");
        sendStatus();
        return;
    }

    Serial.println("Unknown message received");
}

void WebSocketService::onEvent_(WebsocketsEvent event, String data) {
    (void)data;

    if (event == WebsocketsEvent::ConnectionOpened) {
        Serial.println("WebSocket Connected");
        _connected = true;
        return;
    }

    if (event == WebsocketsEvent::ConnectionClosed) {
        Serial.println("WebSocket Closed");
        _connected = false;
        return;
    }
}
