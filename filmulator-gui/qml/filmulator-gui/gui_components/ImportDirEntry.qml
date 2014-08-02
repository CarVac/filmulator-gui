import QtQuick 2.2
import QtQuick.Controls 1.2
import QtQuick.Controls.Styles 1.2
import QtQuick.Dialogs 1.1

Rectangle {
    id: root
    implicitHeight: 50
    implicitWidth: parent.width
    color: "#303030"
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
        color: root.erroneous ? "#FF9922" : "black"
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

        style: ButtonStyle {
            background: Rectangle {
                implicitWidth: 118
                implicitHeight: 23
                border.width: 2
                border.color: "#202020"
                radius: 5
                color: control.pressed ? "#A0A0A0" : "#808080"
            }
            label: Text {
                color: "white"
                anchors.centerIn: parent
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                text: control.text
            }
        }
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
