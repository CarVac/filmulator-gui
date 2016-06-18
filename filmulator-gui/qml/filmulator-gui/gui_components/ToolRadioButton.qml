import QtQuick 2.3
import QtQuick.Controls 1.2
import QtQuick.Controls.Styles 1.2
import "../colors.js" as Colors
import "."

Rectangle {
    id: root
    property real uiScale: 1
    property real __padding: 2 * uiScale
    property alias text: button.text
    property alias checked: button.checked
    property alias tooltipText: tooltip.tooltipText
    property alias exclusiveGroup: button.exclusiveGroup
    property bool standalone: false

    signal pressed()
    signal tooltipWanted(string text, int x, int y)
    width: 30 * uiScale
    height: 30 * uiScale

    color: standalone ? Colors.darkGray : "#00000000"

    RadioButton {
        id: button
        width: parent.width - __padding * 2
        height: parent.height - __padding * 2
        x: __padding
        y: __padding
        style: ToolRadioButtonStyle {
            uiScale: root.uiScale
        }
        Component.onCompleted: {
            button.clicked.connect(root.clicked)
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
