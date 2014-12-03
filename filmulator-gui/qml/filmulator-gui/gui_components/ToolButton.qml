import QtQuick 2.1
import QtQuick.Controls 1.2
import QtQuick.Controls.Styles 1.2
import "../colors.js" as Colors

Item {
    id: root
    property alias text: button.text
    property alias action: button.action
    property alias tooltipText: tooltip.tooltipText
    property real __padding: 2

    signal tooltipWanted(string text, int x, int y)
    width: 30
    height: 30
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
                //border.color: control.pressed ? "#FF9922" : "#A0A0A0"
                border.color: control.pressed ?  Colors.lightOrange : "#A0A0A0"
                radius: 5
                gradient: Gradient {
                    GradientStop {color: control.pressed ? "#000000" : "#222222"; position: 0.0}
                    GradientStop {color: control.pressed ? "#161106" : "#111111"; position: 0.3}
                    GradientStop {color: control.pressed ? "#161106" : "#111111"; position: 0.7}
                    //GradientStop {color: "#111111";                               position: 0.3}
                    //GradientStop {color: "#111111";                               position: 0.7}
                    GradientStop {color: control.pressed ? "#272217" : "#000000"; position: 1.0}
                }
            }
            label: Text {
                color: control.pressed ? Colors.whiteOrange : "white"
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

