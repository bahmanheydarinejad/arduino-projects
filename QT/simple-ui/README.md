# Modern IoT Admin Dashboard (400 x 600)

A lightweight, high-performance Qt Quick (QML + Modern C++) IoT Admin Dashboard application designed specifically for embedded touchscreens, handheld controllers, and compact industrial displays with a portrait **400 x 600** resolution.

---

## 📸 Overview

The dashboard combines a sleek glassmorphic dark theme with real-time telemetry streaming and peripheral actuator controls. It is ready for rapid prototyping using the built-in sensor simulator or for production deployment connected to real microcontrollers (Arduino, ESP32, STM32) over USB Serial, UART, or TCP/IP.

```
+------------------------------------------+
|  [⚡ IoT STUDIO]         (●) ONLINE       |
|  Device: ESP32-Node-01   Sync: 18:45:10  |
+------------------------------------------+
|  [ 🌡 24.5 °C ]       [ 💧 55.2 % ]      |
|  Temperature          Humidity           |
|                                          |
|  [ ⚡ 5.04 V ]        [ 📶 -62 dBm ]     |
|  Supply Voltage       WiFi Signal        |
+------------------------------------------+
|  ACTUATOR CONTROLS                       |
|  Relay 1 (Power)       [  OFF  ( ) ]     |
|  Relay 2 (Auxiliary)   [ ( )  ON   ]     |
|  Fan Speed (PWM)       [=====----] 60%   |
|  LED Brightness        [========-] 85%   |
+------------------------------------------+
|  [ 🔄 Reconnect ]  [ 🔁 Sync ]  [ ⚡ Reboot ] |
+------------------------------------------+
```

---

## 📑 Table of Contents

- [📸 Overview](#-overview)
- [✨ Features](#-features)
- [🏗 System Architecture](#-system-architecture)
  - [Backend Properties & API (`IoTBackend`)](#backend-properties--api-iotbackend)
- [📂 Project Directory Structure](#-project-directory-structure)
- [💻 Building in Qt Creator (Development)](#-building-in-qt-creator-development)
  - [Method 1: Using CMake (Recommended)](#method-1-using-cmake-recommended)
  - [Method 2: Using QMake](#method-2-using-qmake)
- [🐳 Containerized Multi-Platform Builds (Docker Compose)](#-containerized-multi-platform-builds-docker-compose)
  - [1. Build for Raspberry Pi 4 (ARM64)](#1-build-for-raspberry-pi-4-arm64)
  - [2. Build for Desktop Linux (x86_64)](#2-build-for-desktop-linux-x86_64)
  - [3. Build for Windows (via Docker)](#3-build-for-windows-via-docker)
  - [4. Build All Targets Simultaneously](#4-build-all-targets-simultaneously)
- [🪟 Native Windows Build (`build-windows.bat`)](#-native-windows-build-build-windowsbat)
- [🍓 Deploying on Raspberry Pi 4](#-deploying-on-raspberry-pi-4)
  - [1. Prerequisites on Raspberry Pi OS](#1-prerequisites-on-raspberry-pi-os-64-bit-bookworm)
  - [2. Copy and Run](#2-copy-and-run)
  - [3. Running in Kiosk / Touchscreen Mode](#3-running-in-kiosk--touchscreen-mode-without-desktop--x11)
  - [4. Auto-Start on Boot (Systemd Service)](#4-auto-start-on-boot-systemd-service)
- [🔌 Hardware Integration (Arduino / ESP32 Serial)](#-hardware-integration-arduino--esp32-serial)
  - [1. Enable Serial in CMakeLists.txt](#1-enable-serial-in-cmakeliststxt)
  - [2. Connect in C++ (`IoTBackend`)](#2-connect-in-c-iotbackend)
  - [3. Send Commands on Actuator Changes](#3-send-commands-on-actuator-changes)
- [📄 License](#-license)

---

## ✨ Features

- **Designed for 400 x 600 Displays**: Tailored for compact portrait LCDs, HDMI touchscreens, and handheld industrial panels.
- **Glassmorphic Obsidian Theme**: Deep obsidian cards, neon status accents, smooth micro-interactions, and high contrast for outdoor or low-light visibility.
- **Real-Time Telemetry Cards**:
  - 🌡 **Temperature** (°C) with dynamic high-temp warnings.
  - 💧 **Humidity** (% RH) with atmospheric status.
  - ⚡ **Supply Voltage** (V) with low-voltage thresholds.
  - 📶 **Signal Strength / RSSI** (dBm) with visual quality indicators.
- **Peripheral & Actuator Controls**:
  - **Relay 1 & Relay 2**: Animated toggle switches with instant state feedback.
  - **Fan Speed (PWM)**: Smooth interactive slider (0% – 100%).
  - **LED Brightness (PWM)**: Smooth interactive slider (0% – 100%).
- **Device Management**:
  - Live Connection Badge with a pulsating heartbeat LED (Online / Offline).
  - Manual Telemetry Synchronization (`Sync`).
  - Link Toggle / Reconnect (`Reconnect`).
  - Remote System Reboot trigger with simulated boot sequence.
  - In-app animated toast notification system for events and alerts.
- **Built-in Telemetry Simulator**: `IoTBackend` includes an internal timer that generates realistic fluctuating sensor values automatically upon launching—no physical hardware required to test the UI.

---

## 🏗 System Architecture

The application adopts a clean separation of concerns between C++ backend logic and the QML presentation layer:

```
┌────────────────────────────────────────────────────────┐
│                   QML Presentation                     │
│  main.qml  │  DashboardCard.qml  │  IoTControlRow.qml  │
└───────────────────────────▲────────────────────────────┘
                            │ Q_PROPERTY bindings & Q_INVOKABLE calls
┌───────────────────────────▼────────────────────────────┐
│                  C++ Engine (Backend)                  │
│       IoTBackend (QObject)  │  main.cpp (Engine)       │
│  - Telemetry Properties     - Hardware / Serial Hooks  │
│  - Actuator Setters         - Background Simulator     │
└────────────────────────────────────────────────────────┘
```

### Backend Properties & API (`IoTBackend`)

| Category | Identifier | Type | Access | Description |
| :--- | :--- | :--- | :--- | :--- |
| **Telemetry** | `temperature` | `double` | Read-only | Ambient temperature in °C |
| | `humidity` | `double` | Read-only | Relative humidity in % |
| | `voltage` | `double` | Read-only | Bus supply voltage in Volts |
| | `rssi` | `int` | Read-only | Wireless signal strength in dBm |
| | `lastUpdated` | `QString` | Read-only | Timestamp of last received packet |
| **Actuators** | `relay1` | `bool` | Read/Write | State of primary power relay |
| | `relay2` | `bool` | Read/Write | State of auxiliary relay |
| | `fanSpeed` | `int` | Read/Write | PWM fan speed (0 to 100) |
| | `brightness` | `int` | Read/Write | PWM LED brightness level (0 to 100) |
| **Status** | `connected` | `bool` | Read-only | Network/hardware connectivity status |
| | `connectionStatus`| `QString` | Read-only | Human-readable connection string |
| | `deviceName` | `QString` | Read-only | Host or target device identifier |
| **Methods** | `toggleRelay1()` | `void` | `Q_INVOKABLE` | Inverts Relay 1 state |
| | `toggleRelay2()` | `void` | `Q_INVOKABLE` | Inverts Relay 2 state |
| | `rebootDevice()` | `void` | `Q_INVOKABLE` | Triggers device restart sequence |
| | `syncData()` | `void` | `Q_INVOKABLE` | Requests immediate telemetry refresh |
| | `toggleConnection()` | `void` | `Q_INVOKABLE` | Simulates connect / disconnect |

---

## 📂 Project Directory Structure

```
simple-ui/
├── CMakeLists.txt              # Primary CMake configuration (Qt 6 & Qt 5 compatible)
├── simple-ui.pro               # Alternative qmake project definition
├── main.cpp                    # Application entry point, QML engine & context setup
├── iotbackend.h                # C++ Backend QObject declarations & API
├── iotbackend.cpp              # C++ Backend logic & telemetry simulator
├── qml.qrc                     # Qt Resource file bundling QML assets into the binary
├── qmldir                      # QML module definition
├── main.qml                    # Root 400x600 viewport, layout, header, footer, toast
├── DashboardCard.qml           # Reusable glassmorphic telemetry metric card
├── IoTControlRow.qml           # Reusable toggle switch / slider row component
│
├── docker-compose.yml          # Multi-platform containerized build orchestrator
├── docker/
│   ├── Dockerfile.rpi4         # ARM64 Debian Bookworm build (matches Raspberry Pi OS)
│   ├── Dockerfile.linux-x64    # x86_64 Ubuntu 24.04 Linux desktop build
│   └── Dockerfile.windows      # Windows x64 MinGW cross-compiler + Wine windeployqt
│
├── build-windows.bat           # Instant native Windows build & packaging script
├── dist/                       # Output directory for standalone binaries
│   ├── rpi4/                   # simple-ui (Linux ARM64 ELF executable)
│   ├── linux-x64/              # simple-ui (Linux x86_64 ELF executable)
│   └── windows/                # simple-ui.exe + all bundled Qt runtime DLLs & plugins
└── README.md                   # Project documentation
```

---

## 💻 Building in Qt Creator (Development)

### Method 1: Using CMake (Recommended)
1. Open **Qt Creator**.
2. Select **File** &rarr; **Open File or Project...** (`Ctrl + O`).
3. Select `CMakeLists.txt`.
4. Choose your configured Qt Kit (Qt 6.2+ or Qt 5.15+).
5. Click **Configure Project**.
6. Press `Ctrl + R` (or the green **Run** button) to build and launch.

### Method 2: Using QMake
1. Open `simple-ui.pro` in Qt Creator.
2. Select your desktop Qt kit and click **Configure Project**.
3. Press `Ctrl + R` to run.

---

## 🐳 Containerized Multi-Platform Builds (Docker Compose)

You can produce standalone, production-ready distribution packages for **Raspberry Pi 4**, **Desktop Linux**, and **Windows** without installing cross-compilers on your host machine.

The project root is mounted read-only (`/src:ro`), and compiled outputs are written directly to host subdirectories under `dist/`:

### 1. Build for Raspberry Pi 4 (ARM64)
Targeting 64-bit Raspberry Pi OS (Debian Bookworm):
```bash
docker compose run --rm rpi4
```
> **Output:** `dist/rpi4/simple-ui` (Linux ARM64 ELF binary)

### 2. Build for Desktop Linux (x86_64)
Targeting standard x86_64 distributions (Ubuntu, Debian, Fedora):
```bash
docker compose run --rm linux-x64
```
> **Output:** `dist/linux-x64/simple-ui` (Linux x86_64 ELF binary)

### 3. Build for Windows (via Docker)
Cross-compiles with MinGW and executes `windeployqt` via Wine:
```bash
docker compose run --rm windows
```
> **Output:** `dist/windows/` (`simple-ui.exe` + bundled Qt DLLs and QML plugins)

### 4. Build All Targets Simultaneously
```bash
docker compose build
docker compose up
```

---

## 🪟 Native Windows Build (`build-windows.bat`)

If you are developing directly on a Windows PC with Qt installed, building via Docker is not required. You can use the included batch script for an instant (~5 second) compilation and packaging workflow:

```cmd
build-windows.bat
```

This script:
1. Automatically discovers local Qt 6.x MinGW, CMake, and Ninja tools.
2. Compiles `simple-ui.exe` in Release mode.
3. Automatically runs `windeployqt.exe` with `--qmldir` to pull all necessary Qt DLLs, graphics plugins, and QML modules.
4. Outputs the complete, standalone package to `dist/windows/`. You can zip this folder and run it on any Windows PC without installing Qt.

---

## 🍓 Deploying on Raspberry Pi 4

### 1. Prerequisites on Raspberry Pi OS (64-bit Bookworm)
Ensure the required runtime libraries are installed on the Raspberry Pi:
```bash
sudo apt update
sudo apt install -y qt6-base-dev qt6-declarative-dev \
    qml6-module-qtquick qml6-module-qtquick-controls \
    qml6-module-qtquick-layouts
```

### 2. Copy and Run
Copy the compiled binary from your development machine to the Pi over SSH:
```bash
scp dist/rpi4/simple-ui pi@<RASPBERRY_PI_IP>:~/simple-ui
```
On the Raspberry Pi:
```bash
chmod +x ~/simple-ui
~/simple-ui
```

### 3. Running in Kiosk / Touchscreen Mode (Without Desktop / X11)
If your Raspberry Pi is running Raspberry Pi OS Lite or you want dedicated full-screen kiosk performance directly on the hardware framebuffer:
```bash
# Direct hardware OpenGL acceleration (EGLFS)
~/simple-ui -platform eglfs

# Linux framebuffer fallback
~/simple-ui -platform linuxfb
```

### 4. Auto-Start on Boot (Systemd Service)
Create a service file at `/etc/systemd/system/iot-dashboard.service`:
```ini
[Unit]
Description=IoT Admin Dashboard
After=network.target

[Service]
Type=simple
User=pi
Environment=QT_QPA_PLATFORM=eglfs
ExecStart=/home/pi/simple-ui
Restart=always
RestartSec=5

[Install]
WantedBy=multi-user.target
```
Enable and start the service:
```bash
sudo systemctl daemon-reload
sudo systemctl enable iot-dashboard.service
sudo systemctl start iot-dashboard.service
```

---

## 🔌 Hardware Integration (Arduino / ESP32 Serial)

To interface this dashboard with real microcontrollers over USB or UART:

### 1. Enable Serial in `CMakeLists.txt`
```cmake
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Gui Quick Qml SerialPort)
target_link_libraries(simple-ui PRIVATE Qt${QT_VERSION_MAJOR}::SerialPort)
```

### 2. Connect in C++ (`IoTBackend`)
In `iotbackend.h`:
```cpp
#include <QSerialPort>

class IoTBackend : public QObject {
    ...
    QSerialPort m_serial;
};
```

In `iotbackend.cpp`:
```cpp
// Open serial connection
m_serial.setPortName("/dev/ttyUSB0"); // Or "COM3" on Windows
m_serial.setBaudRate(QSerialPort::Baud115200);
m_serial.open(QIODevice::ReadWrite);

connect(&m_serial, &QSerialPort::readyRead, this, [this]() {
    QByteArray data = m_serial.readAll();
    // Parse incoming JSON telemetry: {"temp":23.4,"hum":48.2,"volt":5.01,"rssi":-68}
    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (!doc.isNull() && doc.isObject()) {
        QJsonObject obj = doc.object();
        m_temperature = obj["temp"].toDouble();
        m_humidity = obj["hum"].toDouble();
        m_voltage = obj["volt"].toDouble();
        m_rssi = obj["rssi"].toInt();
        emit telemetryChanged();
    }
});
```

### 3. Send Commands on Actuator Changes
```cpp
void IoTBackend::setRelay1(bool on) {
    if (m_relay1 != on) {
        m_relay1 = on;
        emit relay1Changed(m_relay1);
        if (m_serial.isOpen()) {
            m_serial.write(on ? "CMD:RELAY1:1\n" : "CMD:RELAY1:0\n");
        }
    }
}
```

---

## 📄 License

This project is licensed under the MIT License — feel free to customize, modify, and distribute for personal or commercial embedded IoT products.
