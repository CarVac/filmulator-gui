import QtQuick 2.1
import QtQuick.Controls 1.2
import QtQuick.Controls.Styles 1.2

Rectangle {
    id: root
    implicitHeight: 30
    implicitWidth: parent.width
    property alias title: label.text
    property alias minimumValue: slider.minimumValue
    property alias maximumValue: slider.maximumValue
    property alias updateValueWhileDragging: slider.updateValueWhileDragging
    property alias stepSize: slider.stepSize
    property alias value: slider.value
    property real defaultValue
    property alias valueText: valueText.text
    property alias pressed: slider.pressed
    property alias tooltipText: toolTooltip.tooltipText

    property alias tickmarksEnabled: slider.tickmarksEnabled

    property real __padding: 2

    signal tooltipWanted(string text, int coordX, int coordY)
    signal released()

    onPressedChanged: {
        if (pressed == false) {
            released()
        }
    }

    color: "#303030"

    Text {
        id: label
        color: "white"
        width: parent.width - valueBox.width - reset.width- 4*__padding
        x: __padding
        y: __padding * 1.5
        elide: Text.ElideRight
    }
    Rectangle {
        id: valueBox
        color: "black"
        width: 60
        height: 20 - __padding
        x: parent.width - this.width - reset.width - __padding * 1.5
        y: __padding * 1.5
        Text {
            id: valueText
            x: __padding / 2
            y: __padding / 2
            width: parent.width - x
            height: parent.height - y
            color: "white"
            text: slider.value
            elide: Text.ElideRight
        }
    }

    Slider {
        id: slider
        x: __padding
        y: 20
        width: parent.width - reset.width - 2.5*__padding
        updateValueWhileDragging: true
        value: defaultValue
        style: SliderStyle {
            groove: Rectangle {
                height: 4
                color: "#FF8800"
            }
            handle: Rectangle {
                height: 8
                width: 20
                radius: 3
                color: control.pressed ? "#A0A0A0" : "#808080"
            }
        }
    }
    Button {
        id: reset
        width: 28
        height: 28
        x: root.width-width-__padding/2
        y: __padding/2
        text: "[]"
        action: Action {
            onTriggered: {
                slider.value = defaultValue
            }
        }

        style: ButtonStyle {
            background: Rectangle {
                implicitWidth: 26
                implicitHeight: 26
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
