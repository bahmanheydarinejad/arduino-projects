import QtQuick 2.15

Rectangle {
    id: root

    property string title: "Control"
    property string subtitle: ""
    property string controlType: "switch" // "switch" or "slider"
    property bool checked: false
    property int value: 0               // 0 to 100
    property color accentColor: "#00E5FF"

    signal toggled(bool state)
    signal valueChangedByUser(int newValue)

    implicitWidth: 360
    implicitHeight: root.controlType === "slider" ? 64 : 52
    radius: 10
    color: "#181E29"
    border.color: "#252F3F"
    border.width: 1

    Item {
        anchors.fill: parent
        anchors.margins: 12

        // Left Side: Labels
        Column {
            anchors.left: parent.left
            anchors.verticalCenter: parent.verticalCenter
            spacing: 3
            width: root.controlType === "slider" ? 120 : parent.width - 70

            Text {
                text: root.title
                font.pixelSize: 13
                font.bold: true
                color: "#E2E8F0"
                elide: Text.ElideRight
                width: parent.width
            }

            Text {
                text: root.subtitle
                font.pixelSize: 10
                color: "#7E8E9F"
                visible: text !== ""
                elide: Text.ElideRight
                width: parent.width
            }
        }

        // Right Side: Switch
        Item {
            id: switchWidget
            visible: root.controlType === "switch"
            anchors.right: parent.right
            anchors.verticalCenter: parent.verticalCenter
            width: 46
            height: 24

            Rectangle {
                id: track
                anchors.fill: parent
                radius: height / 2
                color: root.checked ? root.accentColor : "#252F3F"
                Behavior on color { ColorAnimation { duration: 180 } }

                Rectangle {
                    id: thumb
                    width: 18
                    height: 18
                    radius: 9
                    y: 3
                    x: root.checked ? parent.width - width - 3 : 3
                    color: root.checked ? "#0B1017" : "#8E9EAF"
                    Behavior on x { NumberAnimation { duration: 180; easing.type: Easing.InOutQuad } }
                    Behavior on color { ColorAnimation { duration: 180 } }
                }
            }

            MouseArea {
                anchors.fill: parent
                cursorShape: Qt.PointingHandCursor
                onClicked: {
                    root.toggled(!root.checked)
                }
            }
        }

        // Right Side: Slider
        Item {
            id: sliderWidget
            visible: root.controlType === "slider"
            anchors.right: parent.right
            anchors.verticalCenter: parent.verticalCenter
            width: 190
            height: 32

            Row {
                anchors.fill: parent
                spacing: 8
                
                // Track & Thumb
                Item {
                    id: sliderBar
                    width: parent.width - 45
                    height: parent.height

                    Rectangle {
                        id: sliderTrack
                        anchors.verticalCenter: parent.verticalCenter
                        width: parent.width
                        height: 6
                        radius: 3
                        color: "#252F3F"

                        Rectangle {
                            height: parent.height
                            radius: 3
                            width: (root.value / 100.0) * parent.width
                            color: root.accentColor
                        }

                        Rectangle {
                            id: sliderKnob
                            width: 16
                            height: 16
                            radius: 8
                            color: "#FFFFFF"
                            border.color: root.accentColor
                            border.width: 2
                            x: Math.max(0, Math.min((root.value / 100.0) * (parent.width - width), parent.width - width))
                            anchors.verticalCenter: parent.verticalCenter
                        }
                    }

                    MouseArea {
                        anchors.fill: parent
                        cursorShape: Qt.PointingHandCursor
                        preventStealing: true

                        function updateFromMouse(mouseX) {
                            var pct = Math.max(0, Math.min(1.0, mouseX / sliderBar.width));
                            var val = Math.round(pct * 100);
                            root.valueChangedByUser(val);
                        }

                        onPressed: updateFromMouse(mouse.x)
                        onPositionChanged: {
                            if (pressed) updateFromMouse(mouse.x)
                        }
                    }
                }

                // Percentage Value Label
                Text {
                    text: root.value + "%"
                    font.pixelSize: 11
                    font.bold: true
                    color: root.accentColor
                    width: 36
                    horizontalAlignment: Text.AlignRight
                    anchors.verticalCenter: parent.verticalCenter
                }
            }
        }
    }
}
