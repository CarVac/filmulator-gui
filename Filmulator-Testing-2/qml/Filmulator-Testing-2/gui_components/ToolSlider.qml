import QtQuick 2.1
import QtQuick.Controls 1.1
import QtQuick.Controls.Private 1.0

Rectangle {
    id: toolSlider
    implicitHeight: 40
    implicitWidth: parent.width
    property alias title: label.text
    property alias minimumValue: range.minValue
    property alias maximumValue: range.maxValue

    property bool updateValueWhileDragging: true
    readonly property alias pressed: mouseArea.pressed
    readonly property alias hovered: mouseArea.containsMouse

    property alias stepSize: range.stepSize
    property alias value: range.value

    property bool activeFocusOnPress: false

    property bool tickmarksEnabled: false
    property int tickmarkStep: 1
    property real tickmarkStart: 0

    property real __padding: 3

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
    }

    //TODO: Replace RangeModel with something not internal and undocumented.
    Item {
        id: range
        property real minValue: -1.0
        property real maxValue: 1.0
        property real value: 0.0
        property real stepSize: 0.0
        property real posAtMin: __padding
        property real posAtMax: toolSlider.width - handle.width - __padding
        property real position: (posAtMax-posAtMin)*(value-minValue)/(maxValue-minValue)

        onPositionChanged: value = valueOfPosition(position);

        function valueOfPosition(inPosition) {
            return (maxValue-minValue)*(inPosition-posAtMin)/(posAtMax-posAtMin)
        }
    }

    Rectangle {
        id: groove
        width: parent.width - __padding*2
        height: 4
        x: __padding
        y: 30 - groove.height/2
        color: "orange"

    }

    Rectangle {
        id: handle
        y: 30 - handle.height/2
        width: 20
        height: 12

        function updatePos() {
            if (updateValueWhileDragging && !mouseArea.drag.active)
                range.position = x - __padding
        }

        onXChanged: updatePos();
    }


    MouseArea {
        id: mouseArea

        hoverEnabled: true

        preventStealing: true

        width: parent.width
        height: 20
        x: 0
        y: 20
        /*Rectangle {
            anchors.fill: parent
            color: "blue"
            opacity: 0.5
        }*/

        property int clickOffset: 0

        function clamp ( val ) {
            return Math.max(range.positionAtMinimum, Math.min(range.positionAtMaximum, val))
        }

        onMouseXChanged: {
            if (pressed) {
                var pos = clamp (mouse.x + clickOffset - handle.width/2)
                handle.x = pos
            }
        }

        onPressed: {
            if (toolSlider.activeFocusOnPress) {
                toolSlider.forceActiveFocus();
            }

            var point = mouseArea.mapToItem(handle, mouse.x, mouse.y);

            if (handle.contains(Qt.point(point.x, point.y))) {
                clickOffset = handle.width/2 - point.x;
            }
        }

        onReleased: {
            if (!toolSlider.updateValueWhileDragging)
                range.position = handle.x
            clickOffset = 0
        }

    }

}
