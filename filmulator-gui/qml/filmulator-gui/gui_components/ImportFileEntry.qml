import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Dialogs 1.3
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

    onErroneousChanged: {
        if (erroneous) {
            pulseColor.start
        } else {
            pulseColor.complete()
        }
    }

    Rectangle {
        id: textEntryRect
        color: "black"
        SequentialAnimation on color {
            id: pulseColor
            running: root.erroneous
            loops: Animation.Infinite
            ColorAnimation {
                from: "black"
                to: Colors.darkOrange
                duration: 1000
            }
            ColorAnimation {
                from: Colors.darkOrange
                to: "black"
                duration: 1000
            }
        }

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
        background: Rectangle {
            implicitWidth: parent.width
            implicitHeight: parent.width
            border.width: 1 * uiScale
            border.color: parent.pressed ? Colors.lightOrange : Colors.brightGray
            radius: 5 * uiScale
            gradient: Gradient {
                GradientStop {color: openFileButton.pressed ? "#000000" : "#222222"; position: 0.0}
                GradientStop {color: openFileButton.pressed ? "#161106" : "#111111"; position: 0.3}
                GradientStop {color: openFileButton.pressed ? "#161106" : "#111111"; position: 0.7}
                GradientStop {color: openFileButton.pressed ? "#272217" : "#000000"; position: 1.0}
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
