import QtQuick 2.1
import QtQuick.Controls 1.1
import QtQuick.Controls.Styles 1.1

Rectangle {
    id: toolSlider
    implicitHeight: 30
    implicitWidth: parent.width
    property alias title: label.text
    property alias minimumValue: slider.minimumValue
    property alias maximumValue: slider.maximumValue
    property alias updateValueWhileDragging: slider.updateValueWhileDragging
    property alias stepSize: slider.stepSize
    property alias value: slider.value
    property alias valueText: valueText.text

    property alias tickmarksEnabled: slider.tickmarksEnabled

    property real __padding: 2

    color: "#303030"

    Text {
        id: label
        color: "white"
        x: __padding
        y: __padding
    }
    Rectangle {
        id: valueBox
        color: "black"
        width: 80
        height: 20 - __padding
        x: parent.width - this.width - __padding
        y: __padding
        Text {
            id: valueText
            anchors.fill: parent
            color: "white"
            text: slider.value
            elide: Text.ElideRight
        }
    }

    Slider {
        id: slider
        y: 20
        width: parent.width
        updateValueWhileDragging: true
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
}
