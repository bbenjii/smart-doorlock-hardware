# Smart Door Lock Hardware (ESP32-CAM)

Firmware for an **ESP32-CAM**–based smart door lock device.  
The firmware connects the module to Wi-Fi, exposes a camera HTTP server, maintains a persistent WebSocket connection with the backend, streams camera frames, and reacts to lock control commands while reflecting the lock state via an LED.

---

## Overview

Once powered on, the device:
- Connects to your Wi-Fi network
- Starts an HTTP camera server (MJPEG + snapshots)
- Opens a WebSocket connection to the backend
- Listens for `LOCK`, `UNLOCK`, and `GET_STATUS` commands
- Streams JPEG frames and publishes lock status updates
- Keeps a GPIO-controlled LED synchronized with the lock state

---

## Features

- **Wi-Fi client**
    - Connects using credentials defined in `smart-doorlock-hardware.ino`
    - Prints the assigned IP address on boot

- **WebSocket client**
    - Connects to `WEBSOCKET_BASE_URL` defined in `smartlock_module.h`
    - Uses the `/ws/device` endpoint
    - Sends periodic heartbeat/status updates (every 10 seconds)
    - Immediately publishes status after each command

- **Camera streaming**
    - Streams JPEG frames over WebSocket (~50 ms interval)
    - Hosts an HTTP camera server with:
        - `/` (camera UI)
        - `/stream` (MJPEG stream)
        - `/capture` (single JPEG frame)
        - `/bmp`
        - `/status`

- **Lock state indicator**
    - LED on GPIO 2
    - `HIGH` = locked
    - `LOW` = unlocked

---

## Hardware Requirements

- ESP32 camera module with **PSRAM**
    - Default configuration targets `CAMERA_MODEL_WROVER_KIT`
- On-board LED or external LED connected to **GPIO 2**
- 5V power supply
- USB-to-serial adapter for flashing (if required by your board)

> If you are using a different ESP32-CAM variant, update the
> `CAMERA_MODEL_*` definition in `board_config.h` so the pin mapping matches your hardware.

---

## Software Prerequisites

- Arduino IDE **or** PlatformIO
- Espressif ESP32 board package installed
- Required libraries:
    - `ArduinoWebsockets`
    - `WiFi`, `esp_camera`, and HTTP server libraries (included with ESP32 package)
- Partition scheme with:
    - At least **3 MB app space**
    - **PSRAM enabled**

---

## Setup and Flashing

1. Open `smart-doorlock-hardware.ino` in the Arduino IDE.
2. Set your Wi-Fi credentials near the top of the sketch:
   ```cpp
   const char* ssid = "...";
   const char* password = "...";
