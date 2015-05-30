import QtQuick 2.3
import QtQuick.Controls 1.2
import QtQuick.Layouts 1.1
import "gui_components"
import "generateHistogram.js" as Script

SplitView {
    id: root
    property real uiScale: 1
    //width: 250
    anchors.margins: 3 * uiScale
    property real maxWidth: 500 * uiScale
    Layout.maximumWidth: maxWidth
    Layout.minimumWidth: 250 * uiScale
    orientation: Qt.Vertical

    //Here we set up the properties that let us communicate
    // both the image-specific settings (on reloading of an
    // already processed image), as well as default settings
    // for when the user resets the tool back to 'default'.
    property real defaultExposureComp
    property real defaultWhitepoint
    property real defaultBlackpoint
    property real defaultShadowsY
    property real defaultHighlightsY
    property real defaultFilmSize
    property int defaultHighlightRecovery
    property real defaultLayerMixConst
    property bool defaultCaEnabled
    property real defaultTemperature
    property real defaultTint
    property real defaultVibrance
    property real defaultSaturation
    property bool defaultOverdriveEnabled

    signal tooltipWanted(string text, int x, int y)

    Item {
        width: parent.width
        Layout.minimumHeight: 50 * uiScale
        Layout.maximumHeight: 500 * uiScale
        height: 250 * uiScale
        Canvas {
            id: mainHistoCanvas
            anchors.fill: parent
            property int lineWidth: 1 * uiScale
            property real alpha: 1.0
            property int padding: 5 * uiScale
            canvasSize.width: root.maxWidth
            canvasSize.height: 500 * uiScale

            onWidthChanged: requestPaint()
            Connections {
                target: filmProvider
                onHistFinalChanged: mainHistoCanvas.requestPaint()
            }

            onPaint: Script.generateHistogram(1,this.getContext('2d'),width,height,padding,lineWidth,root.uiScale)

        }
    }

    Item {
        id: toolListItem
        width: parent.width
        Layout.fillHeight: true
        Flickable {
            id: toolList
            width: parent.width
            height: parent.height
            flickableDirection: Qt.Vertical
            clip: true
            contentHeight: toolLayout.height
            boundsBehavior: Flickable.StopAtBounds
            ColumnLayout {
                id: toolLayout
                spacing: 0 * uiScale
                x: 3 * uiScale
                width: toolListItem.width - 6 * uiScale

                Rectangle {
                    id: topSpacer
                    color: "#00000000"//transparent
                    height: 3 * uiScale
                }

                ToolSwitch {
                    id: caSwitch
                    tooltipText: qsTr("Automatically correct directional color fringing.")
                    text: qsTr("CA correction")
                    isOn: paramManager.caEnabled
                    defaultOn: root.defaultCaEnabled
                    onIsOnChanged: {
                        paramManager.caEnabled = isOn
                        paramManager.writeback()
                    }
                    onResetToDefault: {
                        paramManager.caEnabled = isOn
                        paramManager.writeback()
                    }
                    Connections {
                        target: paramManager
                        onCaEnabledChanged: {
                            caSwitch.isOn = paramManager.caEnabled
                        }
                    }
                    Component.onCompleted: {
                        caSwitch.tooltipWanted.connect(root.tooltipWanted)
                    }
                    uiScale: root.uiScale
                }

                ToolSlider {
                    id: highlightRecoverySlider
                    title: qsTr("Highlight Recovery")
                    tooltipText: qsTr("Recover clipped highlights. 1 is useless, 2 recovers monochrome values, and 3-9 extrapolates the colors. 3 works best most of the time, but 9 can work better on skin tones.")
                    minimumValue: 0
                    maximumValue: 9
                    stepSize: 1
                    tickmarksEnabled: true
                    value: paramManager.highlights
                    defaultValue: root.defaultHighlightRecovery
                    onValueChanged: {
                        paramManager.highlights = value
                    }
                    onEditComplete: paramManager.writeback()
                    Connections {
                        target: paramManager
                        onHighlightsChanged: {
                            highlightRecoverySlider.value = paramManager.highlights
                        }
                    }
                    Component.onCompleted: {
                        highlightRecoverySlider.tooltipWanted.connect(root.tooltipWanted)
                    }
                    uiScale: root.uiScale
                }

                ToolSlider {
                    id: temperatureSlider
                    title: qsTr("Temperature")
                    tooltipText: qsTr("Correct the image color for a light source of the indicated Kelvin temperature.")
                    minimumValue: Math.log(2000)
                    maximumValue: Math.log(20000)
                    value: Math.log(paramManager.temperature)
                    defaultValue: Math.log(root.defaultTemperature)
                    valueText: Math.exp(value)
                    onValueChanged: {
                        paramManager.temperature = Math.exp(value)
                    }
                    onEditComplete: paramManager.writeback()
                    Connections {
                        target: paramManager
                        onTemperatureChanged: {
                            temperatureSlider.value = Math.log(paramManager.temperature)
                        }
                    }
                    Component.onCompleted: {
                        temperatureSlider.tooltipWanted.connect(root.tooltipWanted)
                    }
                    uiScale: root.uiScale
                }

                ToolSlider {
                    id: tintSlider
                    title: qsTr("Tint")
                    tooltipText: qsTr("Correct for a green/magenta tinted light source. Positive values are greener, and negative values are magenta.")
                    minimumValue: 0.1
                    maximumValue: 3
                    value: paramManager.tint
                    defaultValue: root.defaultTint
                    onValueChanged: {
                        paramManager.tint = value
                    }
                    onEditComplete: paramManager.writeback()
                    Connections {
                        target: paramManager
                        onTintChanged: {
                            tintSlider.value = paramManager.tint
                        }
                    }
                    Component.onCompleted: {
                        tintSlider.tooltipWanted.connect(root.tooltipWanted)
                    }
                    uiScale: root.uiScale
                }

                ToolSlider {
                    id: exposureCompSlider
                    title: qsTr("Exposure Compensation")
                    tooltipText: qsTr("The amount the program should to over- or under-expose the \"film\" relative to the captured exposure. Analogous to exposure of film in-camera. Usually, adjust this until the pre-filmulator histogram uses the full width.")
                    minimumValue: -5
                    maximumValue: 5
                    stepSize: 1/6
                    tickmarksEnabled: true
                    tickmarkFactor: 6
                    value: paramManager.exposureComp
                    defaultValue: root.defaultExposureComp
                    onValueChanged: {
                        paramManager.exposureComp = value
                    }
                    onEditComplete: paramManager.writeback()
                    Connections {
                        target: paramManager
                        onExposureCompChanged: {
                            exposureCompSlider.value = paramManager.exposureComp
                        }
                    }
                    Component.onCompleted: {
                        exposureCompSlider.tooltipWanted.connect(root.tooltipWanted)
                    }
                    uiScale: root.uiScale
                }

                Canvas {
                    id: preFilmHistoCanvas
                    Layout.fillWidth: true
                    //It seems that since this is in a layout, you can't bind dimensions or locations.
                    // Makes sense, given that the layout is supposed to abstract that away.
                    height: 30 * uiScale
                    property int lineWidth: 1 * uiScale
                    property real alpha: 1.0
                    property int padding: 3 * uiScale

                    canvasSize.width: root.maxWidth
                    canvasSize.height: height

                    onWidthChanged: requestPaint();
                    Connections {
                        target: filmProvider
                        onHistPreFilmChanged: preFilmHistoCanvas.requestPaint()
                    }

                    onPaint: Script.generateHistogram(2,this.getContext('2d'),width,height,padding,lineWidth,root.uiScale)

                    ToolTip {
                        id: preFilmTooltip
                        tooltipText: qsTr("This is a histogram of the input to the film simulation.")
                        Component.onCompleted: {
                            preFilmTooltip.tooltipWanted.connect(root.tooltipWanted)
                        }
                    }
                }

                ToolSlider {
                    id: filmSizeSlider
                    title: qsTr("Film Area")
                    tooltipText: qsTr("Larger sizes emphasize smaller details and flatten contrast; smaller sizes emphasize larger regional contrasts. This has the same effect as film size in real film. If venturing into Medium or Large Format, keep the Drama slider below 40 to prevent overcooking.")
                    minimumValue: 10
                    maximumValue: 300
                    value: Math.sqrt(paramManager.filmArea)
                    defaultValue: Math.sqrt(root.defaultFilmSize)
                    //The following thresholds are 24mmx65mm and twice 6x9cm film's
                    // areas, respectively.
                    valueText: (value*value < 1560) ? "SF" : (value*value < 9408) ? "MF" : "LF"
                    onValueChanged: {
                        paramManager.filmArea = value*value
                    }
                    onEditComplete: paramManager.writeback()
                    Connections {
                        target: paramManager
                        onFilmAreaChanged: {
                            filmSizeSlider.value = Math.sqrt(paramManager.filmArea)
                        }
                    }
                    Component.onCompleted: {
                        filmSizeSlider.tooltipWanted.connect(root.tooltipWanted)
                    }
                    uiScale: root.uiScale
                }

                ToolSlider {
                    id: filmDramaSlider
                    title: qsTr("Drama")
                    tooltipText: qsTr("Pulls down highlights to retain detail. This is the real \"filmy\" effect. This not only helps bring down bright highlights, but it can also rescue extremely saturated regions such as flowers.")
                    minimumValue: 0
                    maximumValue: 100
                    value: 100*paramManager.layerMixConst
                    defaultValue: 100*root.defaultLayerMixConst
                    onValueChanged: {
                        paramManager.layerMixConst = value/100;
                    }
                    onEditComplete: paramManager.writeback()
                    Connections {
                        target: paramManager
                        onLayerMixConstChanged: {
                            filmDramaSlider.value = 100*paramManager.layerMixConst
                        }
                    }
                    Component.onCompleted: {
                        filmDramaSlider.tooltipWanted.connect(root.tooltipWanted)
                    }
                    uiScale: root.uiScale
                }

                ToolSwitch {
                    id: overdriveSwitch
                    tooltipText: qsTr("In case of emergency, break glass and press this button. This increases the filminess, in case 100 Drama was not enough for you.")
                    text: qsTr("Overdrive Mode")
                    isOn: (paramManager.agitateCount == 0)
                    defaultOn: root.defaultOverdriveEnabled
                    onIsOnChanged: {
                        paramManager.agitateCount = isOn ? 0 : 1
                        paramManager.writeback()
                    }
                    onResetToDefault: {
                        paramManager.agitateCount = isOn ? 0 : 1
                        paramManager.writeback()
                    }
                    Connections {
                        target: paramManager
                        onAgitateCountChanged: {
                            overdriveSwitch.isOn = (paramManager.agitateCount == 0);
                        }
                    }
                    Component.onCompleted: {
                        overdriveSwitch.tooltipWanted.connect(root.tooltipWanted)
                    }
                    uiScale: root.uiScale
                }

                Canvas {
                    id: postFilmHistoCanvas
                    Layout.fillWidth: true
                    //It seems that since this is in a layout, you can't bind dimensions or locations.
                    // Makes sense, given that the layout is supposed to abstract that away.
                    height: 30 * uiScale
                    property int lineWidth: 1 * uiScale
                    property real alpha: 1.0
                    property int padding: 3 * uiScale

                    canvasSize.width: root.maxWidth
                    canvasSize.height: height

                    onWidthChanged: requestPaint();
                    Connections {
                        target: filmProvider
                        onHistPostFilmChanged: postFilmHistoCanvas.requestPaint()
                    }

                    onPaint: Script.generateHistogram(3,this.getContext('2d'),width,height,padding,lineWidth,root.uiScale)
                    Rectangle {
                        id: blackpointLine
                        height: parent.height
                        width: 1
                        color: blackpointSlider.pressed ? "#FF8800" : "white"
                        x: parent.padding + paramManager.blackpoint/.0025*(parent.width-2*parent.padding)
                        //The .0025 is the highest bin in the post filmulator histogram.
                    }

                    Rectangle {
                        id: whitepointLine
                        height: parent.height
                        width: 1
                        color: whitepointSlider.pressed ? "#FF8800" : "white"
                        x: parent.padding + paramManager.whitepoint/.0025*(parent.width-2*parent.padding)
                        //The .0025 is the highest bin in the post filmulator histogram.
                    }
                    ToolTip {
                        id: postFilmTooltip
                        tooltipText: qsTr("This is a histogram of the output from the film simulation.")
                        Component.onCompleted: {
                            postFilmTooltip.tooltipWanted.connect(root.tooltipWanted)
                        }
                    }
                }

                ToolSlider {
                    id: blackpointSlider
                    title: qsTr("Black Clipping Point")
                    tooltipText: qsTr("This controls the threshold for crushing the shadows. You can see its position in the post-film histogram.")
                    minimumValue: 0
                    maximumValue: 1.4
                    value: Math.sqrt(paramManager.blackpoint*1000)
                    defaultValue: Math.sqrt(root.defaultBlackpoint*1000)
                    valueText: value*value/2
                    onValueChanged: {
                        paramManager.blackpoint = value*value/1000
                    }
                    onEditComplete: paramManager.writeback()
                    Connections {
                        target: paramManager
                        onBlackpointChanged: {
                            blackpointSlider.value = Math.sqrt(paramManager.blackpoint*1000)
                        }
                    }
                    Component.onCompleted: {
                        blackpointSlider.tooltipWanted.connect(root.tooltipWanted)
                    }
                    uiScale: root.uiScale
                }

                ToolSlider {
                    id: whitepointSlider
                    title: qsTr("White Clipping Point")
                    tooltipText: qsTr("This controls the threshold for clipping the highlights. Vaguely analogous to adjusting exposure time in the darkroom. You can see its position in the post-film histogram.")
                    minimumValue: 0.1/1000
                    maximumValue: 2.5/1000
                    value: paramManager.whitepoint
                    defaultValue: root.defaultWhitepoint
                    valueText: value*500// 1000/2
                    onValueChanged: {
                        paramManager.whitepoint = value
                    }
                    onEditComplete: paramManager.writeback()
                    Connections {
                        target: paramManager
                        onWhitepointChanged: {
                            whitepointSlider.value = paramManager.whitepoint
                        }
                    }
                    Component.onCompleted: {
                        whitepointSlider.tooltipWanted.connect(root.tooltipWanted)
                    }
                    uiScale: root.uiScale
                }

                ToolSlider {
                    id: shadowBrightnessSlider
                    title: qsTr("Shadow Brightness")
                    tooltipText: qsTr("This controls the brightness of the generally darker regions of the image.")
                    minimumValue: 0
                    maximumValue: 1
                    value: paramManager.shadowsY
                    defaultValue: root.defaultShadowsY
                    valueText: value*1000
                    onValueChanged: {
                        paramManager.shadowsY = value
                    }
                    onEditComplete: paramManager.writeback()
                    Connections {
                        target: paramManager
                        onShadowsYChanged: {
                            shadowBrightnessSlider.value = paramManager.shadowsY
                        }
                    }
                    Component.onCompleted: {
                        shadowBrightnessSlider.tooltipWanted.connect(root.tooltipWanted)
                    }
                    uiScale: root.uiScale
                }

                ToolSlider {
                    id: highlightBrightnessSlider
                    title: qsTr("Highlight Brightness")
                    tooltipText: qsTr("This controls the brightness of the generally lighter regions of the image.")
                    minimumValue: 0
                    maximumValue: 1
                    value: paramManager.highlightsY
                    defaultValue: root.defaultHighlightsY
                    valueText: value*1000
                    onValueChanged: {
                        paramManager.highlightsY = value
                    }
                    onEditComplete: paramManager.writeback()
                    Connections {
                        target: paramManager
                        onHighlightsYChanged: {
                            highlightBrightnessSlider.value = paramManager.highlightsY
                        }
                    }
                    Component.onCompleted: {
                        highlightBrightnessSlider.tooltipWanted.connect(root.tooltipWanted)
                    }
                    uiScale: root.uiScale
                }

                ToolSlider {
                    id: vibranceSlider
                    title: qsTr("Vibrance")
                    tooltipText: qsTr("This adjusts the vividness of the less-saturated colors in the image.")
                    minimumValue: -0.5
                    maximumValue: 0.5
                    value: paramManager.vibrance
                    defaultValue: root.defaultVibrance
                    valueText: value*200
                    onValueChanged: {
                        paramManager.vibrance = value
                    }
                    onEditComplete: paramManager.writeback()
                    Connections {
                        target: paramManager
                        onVibranceChanged: {
                            vibranceSlider.value = paramManager.vibrance
                        }
                    }
                    Component.onCompleted: {
                        vibranceSlider.tooltipWanted.connect(root.tooltipWanted)
                    }
                    uiScale: root.uiScale
                }

                ToolSlider {
                    id: saturationSlider
                    title: qsTr("Saturation")
                    tooltipText: qsTr("This adjusts the vividness of the entire image.")
                    minimumValue: -0.5
                    maximumValue: 0.5
                    value: paramManager.saturation
                    defaultValue: root.defaultSaturation
                    valueText: value*200
                    onValueChanged: {
                        paramManager.saturation = value
                    }
                    onEditComplete: paramManager.writeback()
                    Connections {
                        target: paramManager
                        onSaturationChanged: {
                            saturationSlider.value = paramManager.saturation
                        }
                    }
                    Component.onCompleted: {
                        saturationSlider.tooltipWanted.connect(root.tooltipWanted)
                    }
                    uiScale: root.uiScale
                }

                Rectangle {
                    id: bottomSpacer
                    color: "#00000000"//transparent
                    height: 3 * uiScale
                }
            }
        }
        MouseArea {
            id: wheelstealer
            //This is to prevent scrolling from adjusting sliders.
            anchors.fill: toolList
            acceptedButtons: Qt.NoButton
            onWheel: {
                if (wheel.angleDelta.y > 0 && !toolList.atYBeginning) {
                    //up
                    toolList.flick(0,600);
                }
                else if (wheel.angleDelta.y < 0 && !toolList.atYEnd) {
                    //down
                    toolList.flick(0,-600);
                }
            }
        }
    }

    Item {
        id: saveButtons
        width: parent.width
        height: 40 * uiScale
        Layout.minimumHeight: 40 * uiScale
        Layout.maximumHeight: 40 * uiScale
        ToolButton {
            id: saveTIFFButton
            width: parent.width/2
            height: 40 * uiScale
            uiScale: root.uiScale
            x: 0
            y: 0
            text: qsTr("Save TIFF")
            action: Action {
                onTriggered: {
                    //filmProvider.saveTiff = true
                    //root.updateImage()
                    filmProvider.writeTiff()
                }
            }
        }
        ToolButton {
            id: saveJPEGButton
            width: parent.width/2
            height: 40 * uiScale
            uiScale: root.uiScale
            x: width
            y: 0
            text: qsTr("Save JPEG")
            action: Action {
                onTriggered: {
                    //filmProvider.saveJpeg = true
                    //root.updateImage()
                    filmProvider.writeJpeg()
                }
            }
        }
    }
}
