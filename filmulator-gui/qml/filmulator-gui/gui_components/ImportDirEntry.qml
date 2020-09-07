import QtQuick 2.12
import QtQuick.Controls 2.12
import Qt.labs.platform 1.0
import "../colors.js" as Colors
import "."

Rectangle {
    id: root
    property real uiScale: 1
    property real __padding: 4 * uiScale
    implicitHeight: 2*__padding + 49 * uiScale
    implicitWidth: parent.width
    property alias title: label.text
    property alias tooltipText: labelTooltip.tooltipText
    property alias warningTooltipText: textEntryTooltip.tooltipText
    property alias dirDialogTitle: dirDialog.title
    property alias enteredText: textEntryBox.text
    property bool erroneous: false
    property bool highlight: false

    color: highlight ? Colors.darkOrangeH : Colors.darkGray

    signal tooltipWanted( string text, int coordX, int coordY )

    Item {
        id: labelBox
        width: parent.width - openDirButton.width - 3*__padding
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

        Image {
            id: errorIcon
            width: 20 * uiScale
            height: 20 * uiScale
            x: parent.width - width - __padding
            y: 4 * uiScale
            source: "qrc:///icons/errortriangle.png"
            antialiasing: true
            visible: root.erroneous
        }

        TextInput {
            id: textEntryBox
            x: __padding
            y: __padding * 1.25
            width: parent.width - x - (root.erroneous ? __padding + errorIcon.width : 0)
            height: parent.height - y
            color: "white"
            selectByMouse: true
            cursorVisible: focus
            font.pixelSize: 12.0 * uiScale
            clip: true
        }
    }

    Button {
        id: openDirButton
        width: 120 * uiScale
        height: 25 * uiScale
        x: root.width - width - __padding
        y: __padding
        text: qsTr( "Select a directory" )
        action: Action {
            onTriggered: {
                dirDialog.folder = textEntryBox.text
                dirDialog.open()
            }
        }
        background: Rectangle {
            implicitWidth: parent.width
            implicitHeight: parent.width
            border.width: 1 * uiScale
            border.color: parent.pressed ? Colors.lightOrange : Colors.brightGray
            radius: 5 * uiScale
            gradient: Gradient {
                GradientStop {color: openDirButton.pressed ? "#000000" : "#222222"; position: 0.0}
                GradientStop {color: openDirButton.pressed ? "#161106" : "#111111"; position: 0.3}
                GradientStop {color: openDirButton.pressed ? "#161106" : "#111111"; position: 0.7}
                GradientStop {color: openDirButton.pressed ? "#272217" : "#000000"; position: 1.0}
            }
        }
        contentItem: Text {
            color: parent.pressed ? Colors.whiteOrange : "white"
            anchors.centerIn: parent
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            text: parent.text
            font.pixelSize: 12.0 * uiScale
        }
    }
    FolderDialog {
        id: dirDialog
        onAccepted: {
            root.enteredText = folder.toString().substring(Qt.platform.os == "windows" ? 8 : 7)
        }
    }

    ToolTip {
        id: labelTooltip
        anchors.fill: labelBox
        Component.onCompleted: labelTooltip.tooltipWanted.connect( root.tooltipWanted )
    }

    ToolTip {
        id: textEntryTooltip
        anchors.fill: textEntryRect
        Component.onCompleted: textEntryTooltip.tooltipWanted.connect(root.tooltipWanted)
        visible: root.erroneous
    }
}
