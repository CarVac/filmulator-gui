import QtQuick 2.1
import QtQuick.Controls 1.2
import QtQuick.Controls.Styles 1.2
import "../colors.js" as Colors
import "."

Rectangle {
    id: root
    implicitHeight: 35
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

    property real __padding: 4

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
        x: parent.width - this.width - reset.width - __padding * 2
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
        y: 20 + __padding
        width: parent.width - reset.width - 3*__padding
        updateValueWhileDragging: true
        value: defaultValue
        style: SliderStyle {
            groove: Rectangle {
                height: 4
                color: "#FF8800"
                gradient: Gradient {
                    GradientStop {color: Colors.brightOrange; position: 0.0}
                    GradientStop {color: Colors.medOrange;   position: 0.3}
                    GradientStop {color: Colors.medOrange;   position: 1.0}
                }
            }
            handle: Rectangle {
                height: 8
                width: 20
                radius: 3
                gradient: Gradient {
                    GradientStop {color: control.pressed ? Colors.brightOrange : Colors.brightGray; position: 0.0}
                    GradientStop {color: control.pressed ? Colors.medOrange    : Colors.middleGray; position: 0.1}
                    GradientStop {color: control.pressed ? Colors.medOrange    : Colors.middleGray; position: 1.0}
                }

                color: control.pressed ? "#A0A0A0" : "#808080"
            }
        }
    }
    Button {
        id: reset
        width: 28
        height: 28
        x: root.width-width-__padding
        y: __padding
        text: "[]"
        action: Action {
            onTriggered: {
                slider.value = defaultValue
            }
        }
        style: ToolButtonStyle {}
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
