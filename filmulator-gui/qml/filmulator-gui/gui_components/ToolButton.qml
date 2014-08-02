import QtQuick 2.1
import QtQuick.Controls 1.2
import QtQuick.Controls.Styles 1.2

Item {
    id: root
    property alias text: button.text
    property alias action: button.action
    property alias tooltipText: tooltip.tooltipText
    property real __padding: 2

    signal tooltipWanted(string text, int x, int y)
    implicitWidth: 30
    implicitHeight: 30
    Button {
        id: button
        width: parent.width - __padding * 2
        height: parent.height - __padding * 2
        x: __padding
        y: __padding
        style: ButtonStyle {
            background: Rectangle {
                implicitWidth: 26
                implicitHeight: 26
                border.width: 2
                border.color: "#A0A0A0"
                radius: 5
                color: control.pressed ? "#606060" : "#000000"
            }
            label: Text {
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
        anchors.fill: button
        Component.onCompleted: {
            //Forward the tooltipWanted signal to root.
            tooltip.tooltipWanted.connect(root.tooltipWanted)
        }
    }
}

