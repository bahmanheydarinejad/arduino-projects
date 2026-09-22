#ifndef IOTBACKEND_H
#define IOTBACKEND_H

#include <QObject>
#include <QTimer>
#include <QDateTime>
#include <QDebug>

class IoTBackend : public QObject
{
    Q_OBJECT

    // Telemetry properties
    Q_PROPERTY(double temperature READ temperature NOTIFY telemetryChanged)
    Q_PROPERTY(double humidity READ humidity NOTIFY telemetryChanged)
    Q_PROPERTY(double voltage READ voltage NOTIFY telemetryChanged)
    Q_PROPERTY(int rssi READ rssi NOTIFY telemetryChanged)

    // Actuator properties
    Q_PROPERTY(bool relay1 READ relay1 WRITE setRelay1 NOTIFY relay1Changed)
    Q_PROPERTY(bool relay2 READ relay2 WRITE setRelay2 NOTIFY relay2Changed)
    Q_PROPERTY(int fanSpeed READ fanSpeed WRITE setFanSpeed NOTIFY fanSpeedChanged)
    Q_PROPERTY(int brightness READ brightness WRITE setBrightness NOTIFY brightnessChanged)

    // Device / Connection state
    Q_PROPERTY(bool connected READ isConnected NOTIFY connectionChanged)
    Q_PROPERTY(QString connectionStatus READ connectionStatus NOTIFY connectionChanged)
    Q_PROPERTY(QString deviceName READ deviceName NOTIFY deviceNameChanged)
    Q_PROPERTY(QString lastUpdated READ lastUpdated NOTIFY telemetryChanged)

public:
    explicit IoTBackend(QObject *parent = nullptr);

    // Getters
    double temperature() const { return m_temperature; }
    double humidity() const { return m_humidity; }
    double voltage() const { return m_voltage; }
    int rssi() const { return m_rssi; }

    bool relay1() const { return m_relay1; }
    bool relay2() const { return m_relay2; }
    int fanSpeed() const { return m_fanSpeed; }
    int brightness() const { return m_brightness; }

    bool isConnected() const { return m_connected; }
    QString connectionStatus() const { return m_connectionStatus; }
    QString deviceName() const { return m_deviceName; }
    QString lastUpdated() const { return m_lastUpdated; }

public slots:
    // Setters
    void setRelay1(bool on);
    void setRelay2(bool on);
    void setFanSpeed(int speed);
    void setBrightness(int val);

    // Action Invokables
    Q_INVOKABLE void toggleRelay1();
    Q_INVOKABLE void toggleRelay2();
    Q_INVOKABLE void rebootDevice();
    Q_INVOKABLE void syncData();
    Q_INVOKABLE void toggleConnection();

signals:
    void telemetryChanged();
    void relay1Changed(bool state);
    void relay2Changed(bool state);
    void fanSpeedChanged(int speed);
    void brightnessChanged(int val);
    void connectionChanged();
    void deviceNameChanged();
    void notification(const QString &title, const QString &message);

private slots:
    void onSimulateTelemetry();

private:
    double m_temperature;
    double m_humidity;
    double m_voltage;
    int m_rssi;

    bool m_relay1;
    bool m_relay2;
    int m_fanSpeed;
    int m_brightness;

    bool m_connected;
    QString m_connectionStatus;
    QString m_deviceName;
    QString m_lastUpdated;

    QTimer m_simTimer;
};

#endif // IOTBACKEND_H
