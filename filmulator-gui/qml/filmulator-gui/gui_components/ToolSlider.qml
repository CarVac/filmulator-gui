import QtQuick 2.12
import QtQuick.Controls 2.12
import "../colors.js" as Colors
import "."

Rectangle {
    id: root
    property real uiScale: 1
    implicitHeight: 36 * uiScale + (parent.width * 0.75 * preciseInputActive)
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
    property alias precisePressed: spinnerCircle.dragging
    property alias tooltipText: toolTooltip.tooltipText

    property bool changed: true
    property bool editMade: false

    property bool highlight: false

    property bool preciseInputActive: false

    property real __padding: 4 * uiScale

    property real __precise

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
        MouseArea {
            id: preciseInputToggle
            anchors.fill: parent
            acceptedButtons: Qt.RightButton
            onClicked: {
                if (stepSize == 0) {
                    preciseInputActive = !preciseInputActive
                }
            }
        }
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
        Image {
            width: 14 * uiScale
            height: 14 * uiScale
            anchors.centerIn: parent
            source: "qrc:///icons/refresh.png"
            antialiasing: true
        }
        action: Action {
            onTriggered: {
                slider.value = defaultValue

                //We have to pretend that the slider was changed
                // so that it writes back to the database.
                root.editComplete()
            }
        }
        background: Rectangle {
            implicitWidth: parent.width
            implicitHeight: parent.width
            border.width: 1 * uiScale
            border.color: parent.pressed ? Colors.lightOrange : Colors.brightGray
            radius: 5 * uiScale
            gradient: Gradient {
                GradientStop {color: reset.pressed ? "#000000" : "#222222"; position: 0.0}
                GradientStop {color: reset.pressed ? "#161106" : "#111111"; position: 0.3}
                GradientStop {color: reset.pressed ? "#161106" : "#111111"; position: 0.7}
                GradientStop {color: reset.pressed ? "#272217" : "#000000"; position: 1.0}
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
    Rectangle {
        id: bottomHighlight
        x: 1 * uiScale
        y: 35 * uiScale
        width: parent.width - 1 * uiScale
        height: parent.height - y
        color: Colors.medOrange
        visible: (stepSize == 0) && preciseInputToggle.containsMouse
    }
    Rectangle {
        id: spinnerCircle
        x: (parent.width - width) / 2 + 1 * uiScale
        //y: (parent.width - bottomHighlight.height) / 2 + 1 * uiScale
        y: 37 * uiScale
        width: bottomHighlight.height - 2 * uiScale
        height: bottomHighlight.height - 2 * uiScale
        radius: width/2
        color: Colors.lowGray
        border.width: 2 * uiScale
        border.color: dragging ? Colors.lightOrange : Colors.brightGray
        visible: preciseInputActive

        property bool dragging: false

        Item {
            id: spinnerNeedleParent
            anchors.centerIn: parent
            width: parent.width
            height: parent.height
            rotation: -180 * preciseInputSpinner.oldAngle / Math.PI
            visible: spinnerCircle.dragging
            Rectangle {
                y: parent.width/2
                x: parent.width/2 - 1.5 * uiScale
                width: 3 * uiScale
                height: parent.height/2
                color: Colors.medOrange
            }
        }

        Rectangle {
            id: innerCircle
            anchors.centerIn: parent
            width: spinnerCircle.width/2.75
            height: spinnerCircle.height/2.75
            radius: width/2
            color: Colors.lowGrayH
            border.width: 2 * uiScale
            border.color: parent.dragging ? Colors.lightOrange : Colors.brightGray
        }

        MouseArea {
            id: preciseInputSpinner
            anchors.fill: parent
            acceptedButtons: Qt.LeftButton
            property real centerX: width/2
            property real centerY: height/2
            property real oldAngle: 0
            onPressed: {
                var x = mouse.x - centerX
                var y = mouse.y - centerY
                if (x^2 + y^2 < width^2) {// it's inside the outer circle
                    spinnerCircle.dragging = true
                    preventStealing = true
                    //Angles are clockwise positive from the top, so I'm doing atan2(x,y) instead of (y,x)
                    oldAngle = Math.atan2(x, y)
                }
            }
            onPositionChanged: {
                var x = mouse.x - centerX
                var y = mouse.y - centerY
                if (spinnerCircle.dragging) {
                    var newAngle = Math.atan2(x, y)
                    var delta = oldAngle - newAngle
                    if ((Math.abs(delta) > Math.PI/2) && (y < 0)) {
                        //if the angle difference is more than pi/2 and the mouse is below the center
                        if (delta < 0) {
                            delta = delta + 2 * Math.PI
                        }
                        else if (delta > 0) {
                            delta = delta - 2 * Math.PI
                        }
                    }
                    oldAngle = newAngle
                    var revolutions = delta / (2 * Math.PI)
                    var newValue = value + revolutions * (maximumValue - minimumValue) / 8
                    value = Math.max(Math.min(newValue,maximumValue),minimumValue)
                }
            }
            onReleased: {
                spinnerCircle.dragging = false
                preventStealing = false
                if (root.editMade) {
                    root.editMade = false
                    root.editComplete()
                }
            }
        }
        ToolTip {
            id: spinnerTooltip
            anchors.fill: innerCircle
            tooltipText: qsTr("Spin clockwise to raise the value.\nSpin counterclockwise to reduce the value.")
            Component.onCompleted: {
                spinnerTooltip.tooltipWanted.connect(root.tooltipWanted)
            }
        }
    }
}
