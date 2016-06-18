import QtQuick 2.3
import QtQuick.Controls 1.2
import QtQuick.Controls.Styles 1.2
import "../colors.js" as Colors
import "."

Rectangle {
    id: root
    property real uiScale: 1
    implicitHeight: 36 * uiScale
    implicitWidth: parent.width
    property alias title: label.text
    property alias minimumValue: slider.minimumValue
    property alias maximumValue: slider.maximumValue
    property alias stepSize: slider.stepSize
    property alias tickmarksEnabled: slider.tickmarksEnabled
    property alias tickmarkFactor: slider.tickmarkFactor
    property alias tickmarkOffset: slider.tickmarkOffset
    property alias minorTicksEnabled: slider.minorTicksEnabled
    property alias value: slider.value
    property real defaultValue
    property alias valueText: valueText.text
    property alias pressed: slider.pressed
    property alias tooltipText: toolTooltip.tooltipText

    property bool changed: true
    property bool editMade: false

    property bool highlight: false

    property real __padding: 4 * uiScale

    signal tooltipWanted(string text, int coordX, int coordY)

    signal editComplete()

    //handler for limiting updates.
    onValueChanged: editMade = true
    onPressedChanged: {
        if (!pressed) {
            if (editMade) {
                editMade = false
                editComplete()
            }
        }
    }

    color: highlight ? Colors.darkOrangeH : Colors.darkGray

    Text {
        id: label
        color: "white"
        width: parent.width - valueBox.width - reset.width- 4*__padding
        x: __padding
        y: __padding * 1.5
        elide: Text.ElideRight
        font.pixelSize: 12.0 * uiScale
    }
    Rectangle {
        id: valueBox
        color: "black"
        width: 60 * uiScale
        height: 21 * uiScale - __padding
        x: parent.width - this.width - reset.width - __padding * 2
        y: __padding * 1.5 - 1 * uiScale
        Text {
            id: valueText
            x: __padding / 2
            y: 1 * uiScale //__padding / 2
            width: parent.width - x
            height: parent.height - y
            color: "white"
            text: slider.value
            elide: Text.ElideRight
            font.pixelSize: 12.0 * uiScale
        }
    }
    SlipperySlider {
        id: slider
        x: __padding
        y: 19 * uiScale + __padding
        width: parent.width - reset.width - 3*__padding
        uiScale: root.uiScale
    }
    Button {
        id: reset
        width: 28 * uiScale
        height: 28 * uiScale
        x: root.width-width-__padding
        y: __padding
        text: "[]"
        action: Action {
            onTriggered: {
                slider.value = defaultValue

                //We have to pretend that the slider was changed
                // so that it writes back to the database.
                root.editComplete()
            }
        }
        style: ToolButtonStyle {
            uiScale: root.uiScale
            notDisabled: root.changed
        }
    }
    MouseArea {
        id: rightclickreset
        acceptedButtons: Qt.RightButton
        anchors.fill: slider
        onDoubleClicked: {
            slider.value = defaultValue
        }
    }
    ToolTip {
        id: toolTooltip
        anchors.fill: label
        Component.onCompleted: {
            //Forward the tooltipWanted signal to root.
            toolTooltip.tooltipWanted.connect(root.tooltipWanted)
        }
    }
    ToolTip {
        id: buttonTooltip
        anchors.fill: reset
        tooltipText: qsTr("Reset to default")
        Component.onCompleted: {
            buttonTooltip.tooltipWanted.connect(root.tooltipWanted)
        }
    }
}
