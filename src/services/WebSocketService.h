#pragma once
#include <Arduino.h>
#include <ArduinoWebsockets.h>

#include "domain/LockController.h"
#include "services/ConfigStore.h"

class WebSocketService {
public:
    explicit WebSocketService(ConfigStore& configStore,
                              LockController& lockController,
                              const char* defaultBaseUrl = "");

    void begin();                  // sets deviceId, builds ws url, registers callbacks
    bool connect(uint32_t timeoutMs = 5000);
    void disconnect();

    void tick();                   // call in loop(): poll + reconnect + periodic status
    bool isConnected() const;

    void sendStatus();             // sends {"type":"status",...}
    void sendCommandFinished(const char* newStatus); // sends {"type":"command_finished",...}

    const String& deviceId() const;

private:
    String buildDeviceId() const;
    String buildWsUrl_(const String& baseUrl) const;
    void refreshWsUrl_();
    void onMessage_(websockets::WebsocketsMessage message);
    void onEvent_(websockets::WebsocketsEvent event, String data);
    void tryReconnect_();

private:
    ConfigStore& _configStore;
    String _baseUrl;
    String _defaultBaseUrl;
    LockController& _lockController;
    
    void smartlockSendCameraFrame();
    websockets::WebsocketsClient _client;

    bool _connected = false;
    String _deviceId;
    String _wsUrl;

    unsigned long _lastReconnectAttempt = 0;
    unsigned long _lastStatusPost = 0;

    const unsigned long _reconnectIntervalMs = 5000;
    const unsigned long _statusIntervalMs = 10000;
};
