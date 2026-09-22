import QtQuick 2.15

Rectangle {
    id: root

    property string title: "Metric"
    property string value: "--"
    property string unit: ""
    property string statusTag: "NORMAL"
    property color accentColor: "#00E5FF"
    property string iconSymbol: "●"

    implicitWidth: 175
    implicitHeight: 100
    radius: 12
    color: "#181E29"
    border.color: "#252F3F"
    border.width: 1

    // Subtle hover/press effect
    MouseArea {
        id: cardArea
        anchors.fill: parent
        hoverEnabled: true
    }

    Rectangle {
        anchors.fill: parent
        radius: parent.radius
        color: root.accentColor
        opacity: cardArea.containsMouse ? 0.05 : 0.0
        Behavior on opacity { NumberAnimation { duration: 150 } }
    }

    Column {
        anchors.fill: parent
        anchors.margins: 12
        spacing: 6

        // Top Row: Icon/Symbol + Title
        Row {
            width: parent.width
            spacing: 6

            Text {
                text: root.iconSymbol
                font.pixelSize: 13
                color: root.accentColor
                anchors.verticalCenter: parent.verticalCenter
            }

            Text {
                text: root.title.toUpperCase()
                font.pixelSize: 10
                font.bold: true
                font.letterSpacing: 0.5
                color: "#7E8E9F"
                anchors.verticalCenter: parent.verticalCenter
            }
        }

        // Middle Row: Big Value + Unit
        Row {
            spacing: 4
            anchors.left: parent.left
            
            Text {
                text: root.value
                font.pixelSize: 22
                font.bold: true
                color: "#F0F4F8"
            }

            Text {
                text: root.unit
                font.pixelSize: 12
                color: "#7E8E9F"
                anchors.bottom: parent.bottom
                anchors.bottomMargin: 3
            }
        }

        // Bottom Row: Status Tag Pill
        Row {
            spacing: 4
            anchors.left: parent.left

            Rectangle {
                width: 6
                height: 6
                radius: 3
                color: root.accentColor
                anchors.verticalCenter: parent.verticalCenter
            }

            Text {
                text: root.statusTag
                font.pixelSize: 9
                font.bold: true
                color: root.accentColor
                anchors.verticalCenter: parent.verticalCenter
            }
        }
    }
}
