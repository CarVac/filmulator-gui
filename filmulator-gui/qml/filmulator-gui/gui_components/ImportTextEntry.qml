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
        height: 20 - __padding
        x: __padding
        y: __padding + 1
        elide: Text.ElideRight
    }
    Rectangle {
        id: textEntryRect
        color: "black"
        width: parent.width - 2*__padding
        height: 30 - 2*__padding
        x: __padding
        y: 20
        TextEdit {
            id: textEntryBox
            color: "white"
            selectByMouse: true
            cursorVisible: focus
            anchors.fill: parent
        }
    }

    ToolTip {
        id: labelTooltip
        anchors.fill: label
        Component.onCompleted: labelTooltip.tooltipWanted.connect( root.tooltipWanted )
    }
}
