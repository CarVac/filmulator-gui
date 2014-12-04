import QtQuick 2.2
import QtQuick.Controls 1.2
import QtQuick.Controls.Styles 1.2
import QtQuick.Dialogs 1.1
import "../colors.js" as Colors
import "."

Rectangle {
    id: root
    implicitHeight: 50
    implicitWidth: parent.width
    color: Colors.darkGray
    property alias title: label.text
    property alias tooltipText: labelTooltip.tooltipText
    property alias dirDialogTitle: dirDialog.title
    property alias enteredText: textEntryBox.text
    property bool erroneous: false

    property real __padding: 2

    signal tooltipWanted( string text, int coordX, int coordY )

    Text {
        id: label
        color: "white"
        width: parent.width - openDirButton.width - 3*__padding
        height: 25 - __padding
        x: __padding
        y: __padding * 1.5
        elide: Text.ElideRight
    }
    Rectangle {
        id: textEntryRect
        color: root.erroneous ? Colors.lightOrange : "black"
        width: parent.width - 2*__padding
        height: 25 - 2*__padding
        x: __padding
        y: 25
        TextEdit {
            id: textEntryBox
            x: __padding
            y: __padding * 1.5
            width: parent.width - x
            height: parent.height - y
            color: root.erroneous ? "black" : "white"
            selectByMouse: true
            cursorVisible: focus
        }
    }

    Button {
        id: openDirButton
        width: 120
        height: 25
        x: root.width - width - __padding / 2
        y: 0//__padding/2
        text: qsTr( "Select a directory" )
        action: Action {
            onTriggered: {
                dirDialog.open()
            }
        }

        style: ToolButtonStyle {}
    }
    FileDialog {
        id: dirDialog
        selectFolder: true
        onAccepted: {
            root.enteredText = fileUrl.toString().substring(7)
        }
    }

    ToolTip {
        id: labelTooltip
        anchors.fill: label
        Component.onCompleted: labelTooltip.tooltipWanted.connect( root.tooltipWanted )
    }
}
