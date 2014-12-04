import QtQuick 2.1
import QtQuick.Controls 1.2
import QtQuick.Controls.Styles 1.2
import "../colors.js" as Colors
import "."

Rectangle {
    id: root
    implicitHeight: 30
    implicitWidth: parent.width
    property alias text: label.text
    property alias tooltipText: tooltip.tooltipText
    property alias isOn: toggleSwitch.checked
    property bool defaultOn

    property real __padding: 2

    signal tooltipWanted(string text, int coordX, int coordY)

    color: Colors.darkGray

    state: defaultOn ? "ON" : "OFF"

    Switch {
        id: toggleSwitch
        x: __padding*2
        anchors.verticalCenter: parent.verticalCenter
        style: SwitchStyle {
            groove: Rectangle {
                implicitWidth: 70
                implicitHeight: 20
                radius: 3
                gradient: Gradient {
                    GradientStop {color: control.checked ? Colors.lightOrange : Colors.brightGrayL; position: 0.0}
                    GradientStop {color: control.checked ? Colors.lightOrange : Colors.brightGray; position: 0.1}
                    GradientStop {color: control.checked ? Colors.lightOrange : Colors.brightGray; position: 1.0}
                }

                border.width: 1
                border.color: control.checked ? Colors.weakOrange : Colors.middleGray
            }
            handle: Rectangle {
                implicitWidth: 30
                implicitHeight: 20
                radius: 3
                gradient: Gradient {
                    GradientStop {color: Colors.lowGrayH; position: 0.0}
                    GradientStop {color: Colors.lowGray; position: 0.15}
                    GradientStop {color: Colors.lowGray; position: 0.9}
                    GradientStop {color: Colors.lowGrayL; position: 1.0}
                }

                border.width: 1
                border.color: control.checked ? Colors.weakOrange : Colors.middleGray
            }
        }
    }

    Text {
        id: label
        width: parent.width - toggleSwitch.width - reset.width - 5*__padding
        x: __padding*5 + toggleSwitch.width
        anchors.verticalCenter: parent.verticalCenter
        color: "white"
        horizontalAlignment: Text.AlignLeft
        verticalAlignment: Text.AlignVCenter
        elide: Text.ElideRight
    }

    Button {
        id: reset
        width: 28
        height: 28
        x: root.width-width-__padding/2
        y: __padding/2
        text: "[]"
        action: Action {
            onTriggered: {
                toggleSwitch.checked = defaultOn
            }
        }

        style: ToolButtonStyle {}
    }

    ToolTip {
        id: tooltip
        anchors.fill: label
        Component.onCompleted: {
            tooltip.tooltipWanted.connect(root.tooltipWanted)
        }
    }
}
