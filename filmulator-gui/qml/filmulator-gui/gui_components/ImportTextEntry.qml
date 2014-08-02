import QtQuick 2.2
import QtQuick.Controls 1.2
import QtQuick.Controls.Styles 1.2

Rectangle {
    id: root
    implicitHeight: 50
    implicitWidth: parent.width
    color: "#303030"
    property alias title: label.text
    property alias tooltipText: labelTooltip.tooltipText
    property alias enteredText: textEntryBox.text

    property real __padding: 2

    signal tooltipWanted( string text, int coordX, int coordY )

    Text {
        id: label
        color: "white"
        width: parent.width - 2*__padding
        height: 25 - __padding
        x: __padding
        y: __padding * 1.5
        elide: Text.ElideRight
    }
    Rectangle {
        id: textEntryRect
        color: "black"
        width: parent.width - 2*__padding
        height: 25 - 2*__padding
        x: __padding
        y: 25
        TextEdit {
            id: textEntryBox
            x: __padding
            y: __padding * 1.5
            color: "white"
            selectByMouse: true
            cursorVisible: focus
        }
    }

    ToolTip {
        id: labelTooltip
        anchors.fill: label
        Component.onCompleted: labelTooltip.tooltipWanted.connect( root.tooltipWanted )
    }
}
