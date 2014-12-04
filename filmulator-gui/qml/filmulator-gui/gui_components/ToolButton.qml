import QtQuick 2.1
import QtQuick.Controls 1.2
import QtQuick.Controls.Styles 1.2
import "../colors.js" as Colors
import "."

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
        style: ToolButtonStyle {}
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

