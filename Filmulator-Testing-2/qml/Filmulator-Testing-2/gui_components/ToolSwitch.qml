import QtQuick 2.1
import QtQuick.Controls 1.1
import QtQuick.Controls.Styles 1.1

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

    color: "#303030"

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
                color: control.checked ? "#FF9922" : "#B0B0B0"
                border.width: 1
                border.color: control.checked ? "#A87848" : "#808080"
            }
            handle: Rectangle {
                implicitWidth: 30
                implicitHeight: 20
                radius: 3
                color: "#606060"
                border.width: 1
                border.color: control.checked ? "#A87848" : "#808080"
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

        style: ButtonStyle {
            background: Rectangle {
                implicitWidth: 26
                implicitHeight: 26
                border.width: 2
                border.color: "#202020"
                radius: 5
                color: control.pressed ? "#A0A0A0" : "#808080"
            }
            label: Text{
                color: "white"
                anchors.centerIn: parent
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                text: control.text
            }
        }
    }

    ToolTip {
        id: tooltip
        anchors.fill: label
        Component.onCompleted: {
            tooltip.tooltipWanted.connect(root.tooltipWanted)
        }
    }
}
