import QtQuick 2.12
import QtQuick.Controls 2.12
import "../colors.js" as Colors
import "."

Rectangle {
    id: root
    property real uiScale: 1
    implicitHeight: 36 * uiScale
    implicitWidth: parent.width
    property alias text: label.text
    property alias tooltipText: tooltip.tooltipText
    property alias isOn: toggleSwitch.checked
    property bool defaultOn: false
    property bool changed: true
    property bool highlight: false
    property alias hovered: hoverStealer.containsMouse

    property real __padding: 4 * uiScale

    signal tooltipWanted(string text, int coordX, int coordY)
    signal resetToDefault()

    color: highlight ? Colors.darkOrangeH : Colors.darkGray

    state: defaultOn ? "ON" : "OFF"

    Switch {
        id: toggleSwitch
        x: __padding*2
        y: __padding/2
        anchors.verticalCenter: parent.verticalCenter
        indicator: Rectangle {
            implicitWidth: 70 * uiScale
            implicitHeight: 20 * uiScale
            radius: 3 * uiScale
            gradient: Gradient {
                GradientStop {color: toggleSwitch.checked ? Colors.lightOrange : Colors.brightGrayL; position: 0.0}
                GradientStop {color: toggleSwitch.checked ? Colors.lightOrange : Colors.brightGray; position: 0.1}
                GradientStop {color: toggleSwitch.checked ? Colors.lightOrange : Colors.brightGray; position: 1.0}
            }

            border.width: 1 * uiScale
            border.color: parent.checked ? Colors.weakOrange : Colors.middleGray

            //handle
            Rectangle {
                x: toggleSwitch.checked ? parent.width-width : 0
                implicitWidth: 30 * uiScale
                implicitHeight: 20 * uiScale
                radius: 3 * uiScale
                gradient: Gradient {
                    GradientStop {color: Colors.lowGrayH; position: 0.0}
                    GradientStop {color: Colors.lowGray; position: 0.15}
                    GradientStop {color: Colors.lowGray; position: 0.9}
                    GradientStop {color: Colors.lowGrayL; position: 1.0}
                }

                border.width: 1 * uiScale
                border.color: toggleSwitch.checked ? Colors.weakOrange : Colors.middleGray
            }
        }
        MouseArea {
            id: hoverStealer
            anchors.fill: parent
            acceptedButtons: Qt.NoButton
            hoverEnabled: true
        }
    }

    Text {
        id: label
        width: parent.width - toggleSwitch.width - reset.width - 5*__padding
        x: __padding*5 + toggleSwitch.width
        y: __padding
        anchors.verticalCenter: parent.verticalCenter
        color: "white"
        horizontalAlignment: Text.AlignLeft
        verticalAlignment: Text.AlignVCenter
        elide: Text.ElideRight
        font.pixelSize: 12.0 * uiScale
    }

    Button {
        id: reset
        width: 28 * uiScale
        height: 28 * uiScale
        x: root.width-width-__padding
        y: __padding
        text: "[]"
        action: Action {
            onTriggered: {
                toggleSwitch.checked = defaultOn
                root.resetToDefault()
            }
        }
        background: Rectangle {
            implicitWidth: parent.width
            implicitHeight: parent.width
            border.width: 1 * uiScale
            border.color: parent.pressed ? Colors.lightOrange : Colors.brightGray
            radius: 5 * uiScale
            gradient: Gradient {
                GradientStop {color: reset.pressed ? "#000000" : "#222222"; position: 0.0}
                GradientStop {color: reset.pressed ? "#161106" : "#111111"; position: 0.3}
                GradientStop {color: reset.pressed ? "#161106" : "#111111"; position: 0.7}
                GradientStop {color: reset.pressed ? "#272217" : "#000000"; position: 1.0}
            }
        }
        contentItem: Text {
            color: parent.pressed ? Colors.whiteOrange : "white"
            anchors.centerIn: parent
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            text: parent.text
            font.pixelSize: 12.0 * uiScale
        }
    }

    ToolTip {
        id: tooltip
        anchors.fill: label
        Component.onCompleted: {
            tooltip.tooltipWanted.connect(root.tooltipWanted)
        }
    }
    ToolTip {
        id: buttonTooltip
        anchors.fill: reset
        tooltipText: qsTr("Reset to default")
        Component.onCompleted: {
            buttonTooltip.tooltipWanted.connect(root.tooltipWanted)
        }
    }
}
