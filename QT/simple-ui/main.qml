import QtQuick 2.15
import QtQuick.Window 2.15

Window {
    id: rootWindow
    width: 400
    height: 600
    minimumWidth: 400
    minimumHeight: 600
    maximumWidth: 400
    maximumHeight: 600
    visible: true
    title: qsTr("IoT Admin Dashboard")
    color: "#0D1117"

    // Toast Notification Banner
    Rectangle {
        id: toast
        anchors.top: parent.top
        anchors.topMargin: 10
        anchors.horizontalCenter: parent.horizontalCenter
        width: Math.min(368, toastText.implicitWidth + 32)
        height: 36
        radius: 18
        color: "#1F6FEB"
        z: 99
        opacity: 0.0
        visible: opacity > 0.0

        Behavior on opacity { NumberAnimation { duration: 250 } }

        Text {
            id: toastText
            anchors.centerIn: parent
            color: "#FFFFFF"
            font.pixelSize: 11
            font.bold: true
        }

        Timer {
            id: toastTimer
            interval: 2200
            onTriggered: toast.opacity = 0.0
        }

        function show(msg) {
            toastText.text = msg;
            toast.opacity = 1.0;
            toastTimer.restart();
        }
    }

    Connections {
        target: backend
        function onNotification(title, message) {
            toast.show(title + ": " + message);
        }
    }

    // Main Content Column
    Item {
        id: container
        anchors.fill: parent
        anchors.margins: 14

        // 1. Header Bar
        Row {
            id: headerRow
            width: parent.width
            height: 48
            spacing: 10

            // Device Icon Badge
            Rectangle {
                width: 44
                height: 44
                radius: 12
                color: "#161B22"
                border.color: "#30363D"
                border.width: 1
                anchors.verticalCenter: parent.verticalCenter

                Text {
                    anchors.centerIn: parent
                    text: "⚡"
                    font.pixelSize: 20
                }
            }

            // Title & Subtitle
            Column {
                anchors.verticalCenter: parent.verticalCenter
                spacing: 3
                width: parent.width - 44 - 10 - statusPill.width - 10

                Text {
                    text: backend ? backend.deviceName : "IoT Node"
                    font.pixelSize: 14
                    font.bold: true
                    color: "#F0F6FC"
                    elide: Text.ElideRight
                    width: parent.width
                }

                Text {
                    text: backend ? backend.connectionStatus : "Connecting..."
                    font.pixelSize: 11
                    color: backend && backend.connected ? "#58A6FF" : "#8B949E"
                    elide: Text.ElideRight
                    width: parent.width
                }
            }

            // Online / Offline Status Pill
            Rectangle {
                id: statusPill
                anchors.verticalCenter: parent.verticalCenter
                width: 78
                height: 28
                radius: 14
                color: backend && backend.connected ? "#1F3526" : "#381E21"
                border.color: backend && backend.connected ? "#2EA043" : "#F85149"
                border.width: 1

                Row {
                    anchors.centerIn: parent
                    spacing: 5

                    Rectangle {
                        width: 6
                        height: 6
                        radius: 3
                        color: backend && backend.connected ? "#3FB950" : "#F85149"
                        anchors.verticalCenter: parent.verticalCenter

                        SequentialAnimation on opacity {
                            running: backend && backend.connected
                            loops: Animation.Infinite
                            PropertyAnimation { to: 0.3; duration: 800 }
                            PropertyAnimation { to: 1.0; duration: 800 }
                        }
                    }

                    Text {
                        text: backend && backend.connected ? "ONLINE" : "OFFLINE"
                        font.pixelSize: 9
                        font.bold: true
                        color: backend && backend.connected ? "#3FB950" : "#F85149"
                        anchors.verticalCenter: parent.verticalCenter
                    }
                }

                MouseArea {
                    anchors.fill: parent
                    cursorShape: Qt.PointingHandCursor
                    onClicked: if (backend) backend.toggleConnection()
                }
            }
        }

        // Divider
        Rectangle {
            id: div1
            anchors.top: headerRow.bottom
            anchors.topMargin: 10
            width: parent.width
            height: 1
            color: "#21262D"
        }

        // 2. Telemetry Grid (2x2)
        Grid {
            id: telemetryGrid
            anchors.top: div1.bottom
            anchors.topMargin: 10
            width: parent.width
            columns: 2
            spacing: 8

            DashboardCard {
                width: (parent.width - 8) / 2
                implicitHeight: 78
                title: "Temperature"
                value: backend ? backend.temperature.toFixed(1) : "--"
                unit: "°C"
                statusTag: "OPTIMAL"
                accentColor: "#FF7B72"
                iconSymbol: "🌡"
            }

            DashboardCard {
                width: (parent.width - 8) / 2
                implicitHeight: 78
                title: "Humidity"
                value: backend ? backend.humidity.toFixed(1) : "--"
                unit: "%"
                statusTag: "NORMAL"
                accentColor: "#79C0FF"
                iconSymbol: "💧"
            }

            DashboardCard {
                width: (parent.width - 8) / 2
                implicitHeight: 78
                title: "Voltage"
                value: backend ? backend.voltage.toFixed(2) : "--"
                unit: "V"
                statusTag: "STABLE"
                accentColor: "#7EE787"
                iconSymbol: "⚡"
            }

            DashboardCard {
                width: (parent.width - 8) / 2
                implicitHeight: 78
                title: "Signal (RSSI)"
                value: backend ? backend.rssi.toString() : "--"
                unit: "dBm"
                statusTag: "GOOD"
                accentColor: "#D2A8FF"
                iconSymbol: "📶"
            }
        }

        // Section Title: Actuators
        Row {
            id: actuatorsLabel
            anchors.top: telemetryGrid.bottom
            anchors.topMargin: 12
            spacing: 6

            Text {
                text: "CONTROL PERIPHERALS"
                font.pixelSize: 10
                font.bold: true
                font.letterSpacing: 0.8
                color: "#8B949E"
            }
        }

        // 3. Actuators Column
        Column {
            id: actuatorsCol
            anchors.top: actuatorsLabel.bottom
            anchors.topMargin: 8
            width: parent.width
            spacing: 7

            // Relay 1 Switch
            IoTControlRow {
                width: parent.width
                implicitHeight: 46
                title: "Relay 1 (Main Light)"
                subtitle: backend && backend.relay1 ? "Active (Closed)" : "Standby (Open)"
                controlType: "switch"
                checked: backend ? backend.relay1 : false
                accentColor: "#58A6FF"
                onToggled: if (backend) backend.toggleRelay1()
            }

            // Relay 2 Switch
            IoTControlRow {
                width: parent.width
                implicitHeight: 46
                title: "Relay 2 (Exhaust Fan)"
                subtitle: backend && backend.relay2 ? "Active (Closed)" : "Standby (Open)"
                controlType: "switch"
                checked: backend ? backend.relay2 : false
                accentColor: "#3FB950"
                onToggled: if (backend) backend.toggleRelay2()
            }

            // Fan Speed Slider (PWM)
            IoTControlRow {
                width: parent.width
                implicitHeight: 52
                title: "Fan PWM Speed"
                controlType: "slider"
                value: backend ? backend.fanSpeed : 0
                accentColor: "#58A6FF"
                onValueChangedByUser: if (backend) backend.setFanSpeed(newValue)
            }

            // LED Brightness Slider (PWM)
            IoTControlRow {
                width: parent.width
                implicitHeight: 52
                title: "LED Brightness"
                controlType: "slider"
                value: backend ? backend.brightness : 0
                accentColor: "#FFA657"
                onValueChangedByUser: if (backend) backend.setBrightness(newValue)
            }
        }

        // 4. Footer System Bar
        Rectangle {
            anchors.bottom: parent.bottom
            width: parent.width
            height: 48
            color: "transparent"

            Row {
                anchors.left: parent.left
                anchors.verticalCenter: parent.verticalCenter
                spacing: 8

                // Sync Button
                Rectangle {
                    width: 78
                    height: 32
                    radius: 8
                    color: "#21262D"
                    border.color: "#30363D"
                    border.width: 1

                    Row {
                        anchors.centerIn: parent
                        spacing: 4
                        Text { text: "⟳"; font.pixelSize: 12; color: "#C9D1D9" }
                        Text { text: "Sync"; font.pixelSize: 11; font.bold: true; color: "#C9D1D9" }
                    }

                    MouseArea {
                        anchors.fill: parent
                        cursorShape: Qt.PointingHandCursor
                        onClicked: if (backend) backend.syncData()
                    }
                }

                // Reboot Button
                Rectangle {
                    width: 82
                    height: 32
                    radius: 8
                    color: "#2A181A"
                    border.color: "#672027"
                    border.width: 1

                    Row {
                        anchors.centerIn: parent
                        spacing: 4
                        Text { text: "⚠"; font.pixelSize: 10; color: "#F85149" }
                        Text { text: "Reboot"; font.pixelSize: 11; font.bold: true; color: "#F85149" }
                    }

                    MouseArea {
                        anchors.fill: parent
                        cursorShape: Qt.PointingHandCursor
                        onClicked: if (backend) backend.rebootDevice()
                    }
                }
            }

            // Last Updated Time Text
            Text {
                anchors.right: parent.right
                anchors.verticalCenter: parent.verticalCenter
                text: "Sync: " + (backend ? backend.lastUpdated : "--:--:--")
                font.pixelSize: 10
                color: "#8B949E"
            }
        }
    }
}
