import QtQuick 2.12
import QtQuick.Controls 2.12
import "../colors.js" as Colors
import "."

Item {
    id: root
    property real uiScale: 1
    property alias text: button.text
    property bool notDisabled: true
    property alias tooltipText: tooltip.tooltipText
    property real __padding: 2 * uiScale
    property bool pressed: notDisabled ? button.pressed : false

    signal triggered()

    signal tooltipWanted(string text, int x, int y)
    width: 30 * uiScale
    height: 30 * uiScale
    Button {
        id: button
        width: parent.width - __padding * 2
        height: parent.height - __padding * 2
        x: __padding
        y: __padding
        action: Action {
            onTriggered: {
                if (root.notDisabled){
                    root.triggered()
                }
            }
        }
        background: Rectangle {
            implicitWidth: 26 * uiScale
            implicitHeight: 26 * uiScale
            border.width: 1 * uiScale
            border.color: notDisabled ? parent.pressed ?  Colors.lightOrange : Colors.brightGray : Colors.brightGray
            radius: 5 * uiScale
            gradient: Gradient {
                GradientStop {color: parent.pressed ? "#000000" : "#222222"; position: 0.0}
                GradientStop {color: parent.pressed ? "#161106" : "#111111"; position: 0.3}
                GradientStop {color: parent.pressed ? "#161106" : "#111111"; position: 0.7}
                GradientStop {color: parent.pressed ? "#272217" : "#000000"; position: 1.0}
            }
        }
        contentItem: Text {
            color: notDisabled ? (parent.pressed ? Colors.whiteOrange : "white") : Colors.middleGray
            anchors.centerIn: parent
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            text: parent.text
            font.pixelSize: 12.0 * uiScale
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

