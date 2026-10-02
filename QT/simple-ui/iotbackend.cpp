#include "iotbackend.h"
#include <QRandomGenerator>
#include <algorithm>

IoTBackend::IoTBackend(QObject *parent)
    : QObject(parent),
      m_temperature(24.6),
      m_humidity(48.2),
      m_voltage(3.31),
      m_rssi(-62),
      m_relay1(false),
      m_relay2(false),
      m_fanSpeed(40),
      m_brightness(80),
      m_connected(true),
      m_connectionStatus("ONLINE (COM3)"),
      m_deviceName("Arduino Nano 33 BLE"),
      m_lastUpdated(QDateTime::currentDateTime().toString("hh:mm:ss"))
{
    // Start periodic background telemetry simulation
    connect(&m_simTimer, &QTimer::timeout, this, &IoTBackend::onSimulateTelemetry);
    m_simTimer.start(2000);
}

void IoTBackend::setRelay1(bool on)
{
    if (m_relay1 != on) {
        m_relay1 = on;
        emit relay1Changed(m_relay1);
        qDebug() << "[Backend] Relay 1 set to:" << (m_relay1 ? "ON" : "OFF");
    }
}

void IoTBackend::setRelay2(bool on)
{
    if (m_relay2 != on) {
        m_relay2 = on;
        emit relay2Changed(m_relay2);
        qDebug() << "[Backend] Relay 2 set to:" << (m_relay2 ? "ON" : "OFF");
    }
}

void IoTBackend::setFanSpeed(int speed)
{
    int clamped = std::clamp(speed, 0, 100);
    if (m_fanSpeed != clamped) {
        m_fanSpeed = clamped;
        emit fanSpeedChanged(m_fanSpeed);
        qDebug() << "[Backend] Fan speed set to:" << m_fanSpeed << "%";
    }
}

void IoTBackend::setBrightness(int val)
{
    int clamped = std::clamp(val, 0, 100);
    if (m_brightness != clamped) {
        m_brightness = clamped;
        emit brightnessChanged(m_brightness);
        qDebug() << "[Backend] LED Brightness set to:" << m_brightness << "%";
    }
}

void IoTBackend::toggleRelay1()
{
    setRelay1(!m_relay1);
}

void IoTBackend::toggleRelay2()
{
    setRelay2(!m_relay2);
}

void IoTBackend::rebootDevice()
{
    qDebug() << "[Backend] Device reboot requested";
    emit notification("Rebooting", "Sending reboot command to " + m_deviceName);
    m_connected = false;
    m_connectionStatus = "REBOOTING...";
    emit connectionChanged();

    // Reconnect simulation after 2.5 seconds
    QTimer::singleShot(2500, this, [this]() {
        m_connected = true;
        m_connectionStatus = "ONLINE (COM3)";
        emit connectionChanged();
        emit notification("Online", m_deviceName + " is back online.");
    });
}

void IoTBackend::syncData()
{
    qDebug() << "[Backend] Sync data requested";
    m_lastUpdated = QDateTime::currentDateTime().toString("hh:mm:ss");
    emit telemetryChanged();
    emit notification("Sync Complete", "Telemetry synced with device.");
}

void IoTBackend::toggleConnection()
{
    m_connected = !m_connected;
    if (m_connected) {
        m_connectionStatus = "ONLINE (COM3)";
        m_simTimer.start(2000);
    } else {
        m_connectionStatus = "DISCONNECTED";
        m_simTimer.stop();
    }
    emit connectionChanged();
    qDebug() << "[Backend] Connection toggled:" << m_connectionStatus;
}

void IoTBackend::onSimulateTelemetry()
{
    if (!m_connected) return;

    // Small random perturbations to simulate real sensor readings
    double tempDelta = (QRandomGenerator::global()->bounded(20) - 10) / 10.0 * 0.15;
    m_temperature = std::clamp(m_temperature + tempDelta, 18.0, 35.0);

    double humDelta = (QRandomGenerator::global()->bounded(20) - 10) / 10.0 * 0.3;
    m_humidity = std::clamp(m_humidity + humDelta, 30.0, 80.0);

    double voltDelta = (QRandomGenerator::global()->bounded(6) - 3) * 0.01;
    m_voltage = std::clamp(3.31 + voltDelta, 3.20, 3.35);

    int rssiDelta = QRandomGenerator::global()->bounded(5) - 2;
    m_rssi = std::clamp(m_rssi + rssiDelta, -85, -45);

    m_lastUpdated = QDateTime::currentDateTime().toString("hh:mm:ss");

    emit telemetryChanged();
}
