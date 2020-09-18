import QtQuick 2.12
import QtQuick.Controls 2.12
import "../colors.js" as Colors
import "."

Rectangle {
    id: root
    property real uiScale: 1
    property real __padding: 2 * uiScale
    property alias text: button.text
    property alias checked: button.checked
    property alias tooltipText: tooltip.tooltipText
    property alias hovered: tooltip.hovered
    property bool standalone: false
    property bool highlight: false

    signal clicked()
    signal tooltipWanted(string text, int x, int y)
    width: 30 * uiScale
    height: 30 * uiScale

    color: standalone ? highlight ? Colors.darkOrangeH : Colors.darkGray : "#00000000"

    RadioButton {
        id: button
        width: parent.width - __padding * 2
        height: parent.height - __padding * 2
        x: __padding
        y: __padding

        ButtonGroup.group: parent.ButtonGroup.group
        Component.onCompleted: {
            button.clicked.connect(root.clicked)
        }
        indicator: Rectangle {
            implicitWidth: parent.width
            implicitHeight: parent.height
            border.width: 1 * uiScale
            border.color: parent.checked ?  Colors.lightOrange : Colors.brightGray
            radius: 5 * uiScale
            gradient: Gradient {
                GradientStop {color: (button.pressed || button.checked) ? "#000000" : "#222222"; position: 0.0}
                GradientStop {color: (button.pressed || button.checked) ? "#161106" : "#111111"; position: 0.3}
                GradientStop {color: (button.pressed || button.checked) ? "#161106" : "#111111"; position: 0.7}
                GradientStop {color: (button.pressed || button.checked) ? "#272217" : "#000000"; position: 1.0}
            }
        }
        contentItem: Text {
            color: (parent.pressed || parent.checked) ? Colors.whiteOrange : Colors.brightGrayH
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
