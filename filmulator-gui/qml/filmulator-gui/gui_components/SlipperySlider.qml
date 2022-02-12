import QtQuick 2.12
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
    height: 12 * uiScale

    property bool sliderEnabled: true

    /*!
        \qmlproperty enumeration SlipperySlider::orientation

        This property holds the layout orientation of the slider.
        The default value is \c Qt.Horizontal.
    */
    //property int orientation: Qt.Horizontal

    /*!
        \qmlproperty real SlipperySlider::minimumValue

        This property holds the minimum value of the slider.
        The default value is \c{0.0}.
    */
    property real minimumValue: 0.0

    /*!
        \qmlproperty real SlipperySlider::maximumValue

        This property holds the maximum value of the slider.
        The default value is \c{1.0}.
    */
    property real maximumValue: 1.0

    /*! \internal */
    property real valRange: maximumValue-minimumValue

    /*!
        \qmlproperty bool SlipperySlider::updateValueWhileDragging

        This property indicates whether the current \l value should be updated while
        the user is moving the slider handle, or only when the button has been released.
        This property could for instance be modified if changing the slider value would turn
        out to be too time consuming.

        The default value is \c true.
    */
    //property bool updateValueWhileDragging: true

    /*!
        \qmlproperty bool SlipperySlider::pressed

        This property indicates whether the slider handle is being pressed.
    */
    readonly property alias pressed: mouseArea.pressed

    /*!
        \qmlproperty bool SlipperySlider::hovered

        This property indicates whether the slider handle is being hovered.
    */
    readonly property alias hovered: mouseArea.handleHovered

    /*!
        \qmlproperty real SlipperySlider::stepSize

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
        \qmlproperty int SlipperySlider::tickmarkFactor

        This property indicates how often a step gets a tickmark.

        A value of 1 indicates that every step gets a tick.

        The default value is \c{1}.
    */
    property int tickmarkFactor: 1

    /*!
        \qmlproperty int SlipperySlider::tickmarkOffset

        This property indicates which is the first step that gets a tickmark.

        A value of 0 indicates that the tickmarks start from the very first.

        The default value is \c{0}.
    */
    property int tickmarkOffset: 0

    /*!
        \qmlproperty bool SlipperySlider::minorTicksEnabled

        This property indicates whether minor tickmarks are enabled.

        The default value is \c false.
    */
    property bool minorTicksEnabled: false

    /*!
        \qmlproperty real SlipperySlider::value

        This property holds the current value of the slider.
        The default value is \c{0.0}.
    */
    property real value: 0

    /*!
        \qmlproperty bool SlipperySlider::activeFocusOnPress

        This property indicates whether the slider should receive active focus when
        pressed.
    */
    property bool activeFocusOnPress: false

    /*!
        \qmlproperty bool SlipperySlider::tickmarksEnabled

        This property indicates whether the slider should display tickmarks
        at step intervals. Tick mark spacing is calculated based on the
        \l stepSize property.

        The default value is \c false.

        \note This property may be ignored on some platforms when using the native style (e.g. Android).
    */
    property bool tickmarksEnabled: false

    property bool boldTickEnabled: false

    property real boldValue: 0

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
        y: 4 * uiScale
        width: parent.width
        height: 4 * uiScale
        gradient: Gradient {
            GradientStop {color: slider.sliderEnabled ? Colors.brightOrange : Colors.brightGray; position: 0.0}
            GradientStop {color: slider.sliderEnabled ? Colors.medOrange    : Colors.middleGray; position: 0.3}
            GradientStop {color: slider.sliderEnabled ? Colors.medOrange    : Colors.middleGray; position: 1.0}
        }
        Row {
            id: tickmarkLayout
            x: posAtMinimum + fakeHandle.width/2 - 1 * uiScale
            width: posRange
            y: 1 * uiScale
            height: 2 * uiScale
            visible: (stepSize > 0) && tickmarksEnabled
            spacing: posRange * (1/(valRange/stepSize)) - 2 * uiScale
            Repeater {
                id: tickmarkRepeater
                model: Math.floor(valRange/stepSize) + 1
                Rectangle {
                    id: tickRect
                    width: 2 * uiScale
                    height: 2 * uiScale
                    radius: 1 * uiScale
                    //You have to add the tickmarkFactor to it because the % operator doesn't work with negative numbers...
                    color: (0>((index+tickmarkFactor-tickmarkOffset+.5)%tickmarkFactor-.5) - posRange/1000) ? Colors.darkGray : (minorTicksEnabled ? Colors.thinDarkGray : "#00000000")
                }
            }
        }
        Rectangle {
            id: boldTickRect
            x: posAtMinimum + fakeHandle.width/2 + (boldValue-minimumValue)*posRange/valRange - width/2
            y: 0.5 * uiScale
            width: 3 * uiScale
            height: 3 * uiScale
            radius: 1 * uiScale
            visible: boldValue >= minimumValue && boldValue <= maximumValue && boldTickEnabled
            color: Colors.darkGray
        }
    }

    Item {
        id: handle
        x: (value-minimumValue)*(posRange)/(valRange)
        y: 0
        height: 12 * uiScale
        width: 20 * uiScale
        Rectangle {
            id: handleRect
            x: 0
            y: 2 * uiScale
            height: 8 * uiScale
            width: 20 * uiScale
            radius: 3 * uiScale
            gradient: Gradient {
                GradientStop {color: slider.pressed ? Colors.brightOrange : slider.hovered ? Colors.weakOrange : Colors.brightGray; position: 0.0}
                GradientStop {color: slider.pressed ? Colors.medOrange    : slider.hovered ? Colors.weakOrange : Colors.middleGray; position: 0.1}
                GradientStop {color: slider.pressed ? Colors.medOrange    : slider.hovered ? Colors.weakOrange : Colors.middleGray; position: 1.0}
            }
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
        enabled: parent.sliderEnabled

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

            //console.log("======================")
            //console.log("overDragThresh")
            //console.log(overDragThresh)
            //console.log("offSliderThresh")
            //console.log(Math.abs(clickOffset2) > fakeHandle.width/2)

            updateHandlePosition(mouse)
        }

        onReleased: {
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
