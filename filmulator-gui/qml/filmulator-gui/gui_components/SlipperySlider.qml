import QtQuick 2.3
import "../colors.js" as Colors

/*! This is a modified implementationof the Qt Quick Controls Slider, without any initial stiction.
  Stickiness made it a royal pain in the butt to make fine tweaks to settings.

  The original code is GPL 2 or later, so I will let this be the same licencing terms.

  This doesn't implement everything.
  It only works in the horizontal orientation.
  It always updates during the drag.
  It doesn't support tickmarks (yet).
  It doesn't support other styles; this may have to be dealt with (sometime).
  It also doesn't necessarily have all the accessibility stuff working.
*/

Item {
    id: slider
    property real uiScale: 1
    width: 200
    height: 8 * uiScale

    /*!
        \qmlproperty enumeration Slider::orientation

        This property holds the layout orientation of the slider.
        The default value is \c Qt.Horizontal.
    */
    //property int orientation: Qt.Horizontal

    /*!
        \qmlproperty real Slider::minimumValue

        This property holds the minimum value of the slider.
        The default value is \c{0.0}.
    */
    property real minimumValue: 0.0

    /*!
        \qmlproperty real Slider::maximumValue

        This property holds the maximum value of the slider.
        The default value is \c{1.0}.
    */
    property real maximumValue: 1.0

    /*! \internal */
    property real valRange: maximumValue-minimumValue

    /*!
        \qmlproperty bool Slider::updateValueWhileDragging

        This property indicates whether the current \l value should be updated while
        the user is moving the slider handle, or only when the button has been released.
        This property could for instance be modified if changing the slider value would turn
        out to be too time consuming.

        The default value is \c true.
    */
    //property bool updateValueWhileDragging: true

    /*!
        \qmlproperty bool Slider::pressed

        This property indicates whether the slider handle is being pressed.
    */
    readonly property alias pressed: mouseArea.pressed

    /*!
        \qmlproperty bool Slider::hovered

        This property indicates whether the slider handle is being hovered.
    */
    readonly property alias hovered: mouseArea.handleHovered

    /*!
        \qmlproperty real Slider::stepSize

        This property indicates the slider step size.

        A value of 0 indicates that the value of the slider operates in a
        continuous range between \l minimumValue and \l maximumValue.

        Any non 0 value indicates a discrete stepSize. The following example
        will generate a slider with integer values in the range [0-5].

        \qml
        Slider {
            maximumValue: 5.0
            stepSize: 1.0
        }
        \endqml

        The default value is \c{0.0}.
    */
    property real stepSize: 0

    /*!
        \qmlproperty real Slider::value

        This property holds the current value of the slider.
        The default value is \c{0.0}.
    */
    property real value: 0

    /*!
        \qmlproperty bool Slider::activeFocusOnPress

        This property indicates whether the slider should receive active focus when
        pressed.
    */
    property bool activeFocusOnPress: false

    /*!
        \qmlproperty bool Slider::tickmarksEnabled

        This property indicates whether the slider should display tickmarks
        at step intervals. Tick mark spacing is calculated based on the
        \l stepSize property.

        The default value is \c false.

        \note This property may be ignored on some platforms when using the native style (e.g. Android).
    */
    //property bool tickmarksEnabled: false

    /*! \internal */
    //property bool __horizontal: true//orientation === Qt.Horizontal

    /*! \internal */
    property real posAtMinimum: 0

    /*! \internal */
    property real posAtMaximum: slider.width - fakeHandle.width

    /*! \internal */
    property real posRange: posAtMaximum-posAtMinimum

    Accessible.role: Accessible.Slider
    /*! \internal */
    function accessibleIncreaseAction() {
        sliderState = "fromValue"
        value = value + stepSize
    }
    /*! \internal */
    function accessibleDecreaseAction() {
        sliderState = "fromValue"
        value = value - stepSize
    }

    property string sliderState: "fromValue"
    onSliderStateChanged: {
        //console.log("slider state:")
        //console.log(sliderState)
        if (sliderState == "fromMouse") {
            fakeHandle.x = handle.x
            value = Qt.binding(function() {return valueHandlePos(fakeHandle.x)})
        }
        else {//if (sliderState == "fromValue") {
            value = value
            fakeHandle.x = handle.x
            fakeHandle.x = Qt.binding(function() {return handle.x})
        }
    }

//    Keys.onRightPressed: if (__horizontal) range.increaseSingleStep()
//    Keys.onLeftPressed: if (__horizontal) range.decreaseSingleStep()
//    Keys.onUpPressed: if (!__horizontal) range.increaseSingleStep()
//    Keys.onDownPressed: if (!__horizontal) range.decreaseSingleStep()

    function clamp(val, minValue, maxValue) {
        return Math.min(maxValue, Math.max(minValue, val))
    }

    function valueHandlePos(handlePos) {
        if (stepSize < (valRange)/1000) {
            return clamp((handlePos - posAtMinimum)*valRange/posRange + minimumValue, minimumValue, maximumValue)
        } else if (handlePos <= posAtMinimum) {
            return minimumValue
        } else if (handlePos >= posAtMaximum) {
            return maximumValue
        } else {
            return clamp(stepSize*Math.round((handlePos - posAtMinimum)*valRange/(stepSize*posRange)) + minimumValue, minimumValue, maximumValue)
        }
    }

    Rectangle {
        id: groove
        x: 0
        y: 2 * uiScale
        width: parent.width
        height: 4 * uiScale
        gradient: Gradient {
            GradientStop {color: Colors.brightOrange; position: 0.0}
            GradientStop {color: Colors.medOrange;    position: 0.3}
            GradientStop {color: Colors.medOrange;    position: 1.0}
        }
    }

    Rectangle {
        id: handle
        x: (value-minimumValue)*(posRange)/(valRange)
        y: 0
        height: 8 * uiScale
        width: 20 * uiScale
        radius: 3 * uiScale
        gradient: Gradient {
            GradientStop {color: slider.pressed ? Colors.brightOrange : slider.hovered ? Colors.weakOrange : Colors.brightGray; position: 0.0}
            GradientStop {color: slider.pressed ? Colors.medOrange    : slider.hovered ? Colors.weakOrange : Colors.middleGray; position: 0.1}
            GradientStop {color: slider.pressed ? Colors.medOrange    : slider.hovered ? Colors.weakOrange : Colors.middleGray; position: 1.0}
        }
    }

    Item {
        id: fakeHandle
        x: (value-minimumValue)*(posRange)/(valRange)
        anchors.verticalCenter: parent.verticalCenter//__horizontal ? parent.verticalCenter : undefined
        width: handle.width
        height: handle.height
        //color: "#AA000055"//turn this into a rectangle and uncomment this line for debugging
    }

    MouseArea {
        id: mouseArea

        anchors.fill: slider
        hoverEnabled: true
        property int clickOffset: 0
        property int clickOffset2: 0
        property real pressX: 0
        property real pressY: 0
        property bool handleHovered: false
        property bool overDragThresh: false

        function clamp ( val ) {
            return Math.max(posAtMinimum, Math.min(posAtMaximum, val))
        }

        function updateHandlePosition(mouse) {
            var pos, overThreshold
            pos = clamp (mouse.x + clickOffset - fakeHandle.width/2)
            var offSliderThresh = Math.abs(clickOffset2) > (fakeHandle.width/2)
            if (overDragThresh || offSliderThresh) {
                slider.sliderState = "fromMouse"
                preventStealing = true
                fakeHandle.x = pos
                value = Qt.binding(function() {return valueHandlePos(fakeHandle.x)})
            }
            else {
                //do nothing
            }
        }

        onPositionChanged: {
            if (pressed) {
                //If it has ever exceeded the drag threshold, it will stay that way (until released)
                overDragThresh = overDragThresh || (Math.abs(mouse.x - pressX) >= 1);
                updateHandlePosition(mouse)
            }

            var point = mouseArea.mapToItem(fakeHandle, mouse.x, mouse.y)
            handleHovered = fakeHandle.contains(Qt.point(point.x, point.y))
        }

        onPressed: {
            preventStealing = true
            if (slider.activeFocusOnPress) {
                slider.forceActiveFocus()
            }

            var point = mouseArea.mapToItem(fakeHandle, mouse.x, mouse.y)
            if (handleHovered) {
                clickOffset = fakeHandle.width/2 - point.x
            }
            clickOffset2 = fakeHandle.width/2 - point.x

            //The location of the initial mouse press, used to detect motion.
            pressX = mouse.x
            pressY = mouse.y

            console.log("======================")
            console.log("overDragThresh")
            console.log(overDragThresh)
            console.log("offSliderThresh")
            console.log(Math.abs(clickOffset2) > fakeHandle.width/2)

            updateHandlePosition(mouse)
        }

        onReleased: {
            updateHandlePosition(mouse)
            if (slider.sliderState !== "fromValue") {
                slider.sliderState = "fromValue"
            }
            clickOffset = 0
            overDragThresh = false
            preventStealing = false
        }

        onExited: handleHovered = false
    }
}
