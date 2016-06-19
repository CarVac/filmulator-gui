import QtQuick 2.3
import QtQuick.Controls 1.2
import QtQuick.Controls.Styles 1.2
import QtQuick.Dialogs 1.2
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
    property alias fileDialogTitle: fileDialog.title
    property alias enteredText: textEntryBox.text
    property alias nameFilters: fileDialog.nameFilters
    property bool erroneous: false
    property bool highlight: false

    color: highlight ? Colors.darkOrangeH : Colors.darkGray

    signal tooltipWanted(string text, int coordX, int coordY)

    Item {
        id: labelBox
        width: parent.width - openFileButton.width - 3*__padding
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
        color: root.erroneous ? Colors.lightOrange : "black"
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
            color: root.erroneous ? "black" : "white"
            selectByMouse: true
            cursorVisible: focus
            font.pixelSize: 12.0 * uiScale
            clip: true
        }
    }

    Button {
        id: openFileButton
        width: 120 * uiScale
        height: 25 * uiScale
        x: root.width - width - __padding
        y: __padding
        text: qsTr("Select files")
        action: Action {
            onTriggered: {
                fileDialog.folder = textEntryBox.text
                fileDialog.open()
            }
        }

        style: ToolButtonStyle {uiScale: root.uiScale}
    }
    FileDialog {
        id: fileDialog
        selectMultiple: true
        onAccepted: {
            root.enteredText = fileUrls.toString()//.substring(7)
        }
    }

    ToolTip {
        id: labelTooltip
        anchors.fill: labelBox
        Component.onCompleted: labelTooltip.tooltipWanted.connect(root.tooltipWanted)
    }

    ToolTip {
        id: textEntryTooltip
        anchors.fill: textEntryRect
        Component.onCompleted: textEntryTooltip.tooltipWanted.connect(root.tooltipWanted)
        visible: root.erroneous
    }
}
