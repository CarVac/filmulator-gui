import QtQuick 2.12
import QtQuick.Controls 2.12
import "../colors.js" as Colors
import "."

Item {
    id: root
    property real uiScale: 1
    width: 200 * uiScale
    height: 30 * uiScale
    property alias value: progressBar.value
    property alias tooltipText: tooltip.tooltipText

    property real __padding: 2 * uiScale

    signal tooltipWanted(string text, int coordX, int coordY)

    ProgressBar {
        id: progressBar
        indeterminate: false
        visible: true
        x: __padding
        y: __padding

        background: Rectangle {
            implicitWidth: root.width - __padding * 2
            implicitHeight: root.height - __padding * 2
            radius: 3 * uiScale
            color: Colors.brightGrayH
            border.width: 1 * uiScale
            border.color: Colors.middleGray
        }
        contentItem: Item {
            implicitWidth: root.width - __padding * 2
            implicitHeight: root.height - __padding * 2
            Rectangle {
                width: progressBar.visualPosition*parent.width
                height: parent.height
                color: Colors.lightOrange
                border.color: Colors.weakOrange
                radius: 3 * uiScale
            }
        }
    }
    ToolTip {
        id: tooltip
        anchors.fill: root
        Component.onCompleted: {
            //forward tooltipWanted to root
            tooltip.tooltipWanted.connect(root.tooltipWanted)
        }
    }
}
