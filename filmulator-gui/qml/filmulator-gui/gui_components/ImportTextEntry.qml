import QtQuick 2.3
import QtQuick.Controls 1.2
import QtQuick.Controls.Styles 1.2
import "../colors.js" as Colors

Rectangle {
    id: root
    property real uiScale: 1
    property real __padding: 4 * uiScale
    implicitHeight: 2*__padding + 49 * uiScale
    implicitWidth: parent.width
    color: Colors.darkGray
    property alias title: label.text
    property alias tooltipText: labelTooltip.tooltipText
    property alias enteredText: textEntryBox.text


    signal tooltipWanted( string text, int coordX, int coordY )

    Item {
        id: labelBox
        width: parent.width - 2*__padding
        height: 25 * uiScale
        x: __padding
        y: __padding
        Text {
            id: label
            color: "white"
            anchors.fill: parent
            verticalAlignment: Text.AlignVCenter
            elide: Text.ElideRight
            font.pixelSize: 12.0 * uiScale
        }
    }
    Rectangle {
        id: textEntryRect
        color: "black"
        width: parent.width - 2*__padding
        height: 25 * uiScale
        x: __padding
        y: 25 * uiScale + __padding
        TextInput {
            id: textEntryBox
            x: __padding
            y: __padding * 1.25
            width: parent.width - x
            height: parent.height - y
            color: "white"
            selectByMouse: true
            cursorVisible: focus
            font.pixelSize: 12.0 * uiScale
            clip: true
        }
    }

    ToolTip {
        id: labelTooltip
        anchors.fill: labelBox
        Component.onCompleted: labelTooltip.tooltipWanted.connect( root.tooltipWanted )
    }
}
