# Modern IoT Admin Dashboard (400 x 600)

A lightweight, modern Qt Quick (QML + C++) IoT Admin Dashboard application designed for embedded displays or touchpanels with a **400 x 600** portrait resolution.

---

## 📱 Features

- **Fixed/Targeted 400x600 Display**: Optimized for compact IoT touchscreens, handheld controllers, or embedded panels.
- **Modern Dark UI**: Obsidian card-based layout with high-contrast status colors.
- **Live Telemetry Dashboard**:
  - 🌡 **Temperature** (°C)
  - 💧 **Humidity** (%)
  - ⚡ **Supply Voltage** (V)
  - 📶 **Signal Strength / RSSI** (dBm)
- **Actuator & Peripheral Controls**:
  - **Relay 1 & Relay 2**: Smooth animated toggle switches with state labels.
  - **Fan Speed (PWM)**: Interactive slider control (0 - 100%).
  - **LED Brightness (PWM)**: Interactive slider control (0 - 100%).
- **Device Management**:
  - Online/Offline status badge with pulsating activity indicator.
  - Reconnect / Toggle connection button.
  - Device Reboot trigger with simulated re-initialization.
  - Telemetry manual Sync button.
  - In-app toast notification system.
- **Built-in Telemetry Simulator**: `IoTBackend` includes an active background simulation timer so the UI comes alive immediately upon launching, before connecting to real hardware.

---

## 📂 Project Structure

```
simple-ui/
├── CMakeLists.txt        # Primary build file (Qt 6 & Qt 5 compatible)
├── simple-ui.pro         # Alternative qmake project file
├── main.cpp              # Application entry point & QML engine setup
├── iotbackend.h          # C++ Backend QObject properties & invokables
├── iotbackend.cpp        # Backend logic & simulation timer
├── qml.qrc               # Qt Resource definition
├── main.qml              # Main 400x600 dashboard window
├── DashboardCard.qml     # Reusable glassmorphic telemetry card
├── IoTControlRow.qml     # Reusable toggle switch / slider component
└── README.md             # Documentation
```

---

## 🚀 How to Run in Qt Creator

### Method 1: Using CMake (Recommended)
1. Open **Qt Creator**.
2. Click **File** > **Open File or Project...** (or press `Ctrl + O`).
3. Select `CMakeLists.txt` in this folder.
4. Select your configured Qt Kit (e.g. Qt 6.x MinGW / MSVC or Qt 5.x).
5. Click **Configure Project**.
6. Press **Ctrl + R** (or click the green Play button) to build and run.

### Method 2: Using QMake
1. In Qt Creator, select `simple-ui.pro`.
2. Configure with your qmake kit and press **Ctrl + R**.

---

## 🔌 Connecting to Real Hardware (Arduino / Serial)

To bridge this UI to an Arduino over USB Serial:
1. Add `serialport` to `CMakeLists.txt`:
   ```cmake
   find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Gui Quick Qml SerialPort)
   target_link_libraries(simple-ui PRIVATE Qt${QT_VERSION_MAJOR}::SerialPort)
   ```
2. In `iotbackend.h`, include `<QSerialPort>`:
   ```cpp
   #include <QSerialPort>
   QSerialPort m_serial;
   ```
3. Read incoming serial JSON packets (e.g. `{"temp":24.5,"hum":50}`) in a `readyRead()` slot and update `m_temperature`, `m_humidity`, etc.
4. Send commands over serial in `toggleRelay1()`, `setFanSpeed()`, etc.:
   ```cpp
   m_serial.write("RELAY1:1\n");
   ```
