import QtQuick 2.2
import QtQuick.Controls 1.2
import QtQuick.Layouts 1.1
import "gui_components"
import "generateHistogram.js" as Script

SplitView {
    id: root
    width: 250
    anchors.margins: 3
    Layout.maximumWidth: 500
    Layout.minimumWidth: 250
    orientation: Qt.Vertical

    //Here we set up the properties that let us communicate
    // both the image-specific settings (on reloading of an
    // already processed image), as well as default settings
    // for when the user resets the tool back to 'default'.
    property real exposureComp
    property real defaultExposureComp
    property real whitepoint
    property real defaultWhitepoint
    property real blackpoint
    property real defaultBlackpoint
    property real shadowsY
    property real defaultShadowsY
    property real highlightsY
    property real defaultHighlightsY
    property real filmSize
    property real defaultFilmSize
    property bool defaultCurve
    property int highlightRecovery
    property int defaultHighlightRecovery
    property real layerMixConst
    property real defaultLayerMixConst
    property bool caEnabled
    property bool defaultCaEnabled
    property real temperature
    property real defaultTemperature
    property real tint
    property real defaultTint
    property real vibrance
    property real defaultVibrance
    property real saturation
    property real defaultSaturation

    signal setAllValues()

    signal updateImage()

    signal tooltipWanted(string text, int x, int y)

    Canvas {
        id:mainHistoCanvas
        width:parent.width
        Layout.minimumHeight: 50
        Layout.maximumHeight: 250
        height:250
        property int lineWidth: 1
        property real alpha: 1.0
        property int padding: 5
        antialiasing: true

        onWidthChanged:requestPaint();
        Connections {
            target: filmProvider
            onHistFinalChanged: mainHistoCanvas.requestPaint();
        }

        onPaint: Script.generateHistogram(1,this.getContext('2d'),width,height,padding,lineWidth)

    }


    Item {
        id: toolListItem
        width: parent.width
        Flickable {
            id: toolList
            width: parent.width
            height: parent.height
            flickableDirection: Qt.Vertical
            clip: true
            contentHeight: toolLayout.height
            ColumnLayout {
                id: toolLayout
                spacing: 0
                x: 3
                width: toolListItem.width - 6

                Rectangle {
                    id: topSpacer
                    color: "#00000000"//transparent
                    height: 3
                }

                ToolSwitch {
                    id: caSwitch
                    tooltipText: qsTr("Automatically correct directional color fringing.")
                    text: qsTr("CA correction")
                    onIsOnChanged: {
                        filmProvider.caEnabled = isOn;
                        root.updateImage()
                    }
                    Connections {
                        target: root
                        onSetAllValues: {
                            caSwitch.isOn = caEnabled;
                        }
                    }
                    Component.onCompleted: {
                        caSwitch.tooltipWanted.connect(root.tooltipWanted)
                        caSwitch.isOn = defaultCaEnabled;
                    }
                }

                ToolSlider {
                    id: temperatureSlider
                    title: qsTr("Temperature")
                    tooltipText: qsTr("Correct the image color for a light source of the indicated Kelvin temperature.")
                    minimumValue: 1500
                    maximumValue: 15000
                    //stepSize: 10
                    defaultValue: root.defaultTemperature
                    onValueChanged: {
                        filmProvider.temperature = value
                        root.updateImage()
                    }
                    Connections {
                        target: root
                        onSetAllValues: {
                            temperatureSlider.value = temperature
                        }
                    }
                    Component.onCompleted: {
                        temperatureSlider.tooltipWanted.connect(root.tooltipWanted)
                    }
                }

                ToolSlider {
                    id: tintSlider
                    title: qsTr("Tint")
                    tooltipText: qsTr("Correct for a green/magenta tinted light source. Positive values are greener, and negative values are magenta.")
                    minimumValue: 0.1
                    maximumValue: 3
                    //stepSize: 0.002
                    defaultValue: root.defaultTint
                    onValueChanged: {
                        filmProvider.tint = value
                        root.updateImage()
                    }
                    Connections {
                        target: root
                        onSetAllValues: {
                            tintSlider.value = tint
                        }
                    }
                    Component.onCompleted: {
                        tintSlider.tooltipWanted.connect(root.tooltipWanted)
                    }
                }

                ToolSlider {
                    id: exposureCompSlider
                    title: qsTr("Exposure Compensation")
                    tooltipText: qsTr("The degree to over- or under-expose the \"film\". Analogous to exposure of film in-camera.")
                    minimumValue: -5
                    maximumValue: 5
                    stepSize: 1/6
                    defaultValue: root.defaultExposureComp
                    onValueChanged: {
                        filmProvider.exposureComp = value
                        root.updateImage()
                    }
                    Connections {
                        target: root
                        onSetAllValues: {
                            exposureCompSlider.value = exposureComp
                        }
                    }
                    Component.onCompleted: {
                        exposureCompSlider.tooltipWanted.connect(root.tooltipWanted)
                    }
                }

                ToolSlider {
                    id: highlightRecoverySlider
                    title: qsTr("Highlight Recovery")
                    tooltipText: qsTr("Recover clipped highlights.")
                    minimumValue: 0
                    maximumValue: 9
                    stepSize: 1
                    defaultValue: root.defaultHighlightRecovery
                    onValueChanged: {
                        filmProvider.highlights = value
                        root.updateImage()
                    }
                    Connections {
                        target: root
                        onSetAllValues: {
                            highlightRecoverySlider.value = highlightRecovery
                        }
                    }
                    Component.onCompleted: {
                        highlightRecoverySlider.tooltipWanted.connect(root.tooltipWanted)
                    }
                }

                Canvas {
                    id: preFilmHistoCanvas
                    Layout.fillWidth: true
                    //It seems that since this is in a layout, you can't bind dimensions or locations.
                    // Makes sense, given that the layout is supposed to abstract that away.
                    height: 30
                    property int lineWidth: 1
                    property real alpha: 1.0
                    property int padding: 3
                    antialiasing: true

                    onWidthChanged: requestPaint();
                    Connections {
                        target: filmProvider
                        onHistPreFilmChanged: preFilmHistoCanvas.requestPaint()
                    }

                    onPaint: Script.generateHistogram(2,this.getContext('2d'),width,height,padding,lineWidth)

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
                    tooltipText: qsTr("Larger values emphasize smaller details; smaller values emphasize large regions.")
                    minimumValue: 10
                    maximumValue: 300
                    defaultValue: Math.sqrt(root.defaultFilmSize)
                    //The following thresholds are 24mmx65mm and twice 6x9cm film's
                    // areas, respectively.
                    valueText: (value*value < 1560) ? "SF" : (value*value < 9408) ? "MF" : "LF"
                    onValueChanged: {
                        filmProvider.filmArea = value*value
                        root.updateImage()
                    }
                    Connections {
                        target: root
                        onSetAllValues: {
                            filmSizeSlider.value = Math.sqrt(filmSize)
                        }
                    }
                    Component.onCompleted: {
                        filmSizeSlider.tooltipWanted.connect(root.tooltipWanted)
                    }
                }

                ToolSlider {
                    id: filmDramaSlider
                    title: qsTr("Drama")
                    tooltipText: qsTr("Pulls down highlights to retain detail.")
                    minimumValue: 0
                    maximumValue: 100
                    defaultValue: 100*root.defaultLayerMixConst
                    onValueChanged: {
                        filmProvider.layerMixConst = value/100;
                        root.updateImage()
                    }
                    Connections {
                        target: root
                        onSetAllValues: {
                            filmDramaSlider.value = 100*layerMixConst
                        }
                    }
                    Component.onCompleted: {
                        filmDramaSlider.tooltipWanted.connect(root.tooltipWanted)
                    }
                }

                Canvas {
                    id: postFilmHistoCanvas
                    Layout.fillWidth: true
                    //It seems that since this is in a layout, you can't bind dimensions or locations.
                    // Makes sense, given that the layout is supposed to abstract that away.
                    height: 30
                    property int lineWidth: 1
                    property real alpha: 1.0
                    property int padding: 3
                    antialiasing: true

                    onWidthChanged: requestPaint();
                    Connections {
                        target: filmProvider
                        onHistPostFilmChanged: postFilmHistoCanvas.requestPaint()
                    }

                    onPaint: Script.generateHistogram(3,this.getContext('2d'),width,height,padding,lineWidth)
                    Rectangle {
                        id: blackpointLine
                        height: parent.height
                        width: 1
                        color: blackpointSlider.pressed ? "#FF8800" : "white"
                        x: parent.padding + filmProvider.blackpoint/.0025*(parent.width-2*parent.padding)
                    }

                    Rectangle {
                        id: whitepointLine
                        height: parent.height
                        width: 1
                        color: whitepointSlider.pressed ? "#FF8800" : "white"
                        x: parent.padding + filmProvider.whitepoint/.0025*(parent.width-2*parent.padding)
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
                    tooltipText: qsTr("The threshold for crushing the shadows.")
                    minimumValue: 0
                    maximumValue: 1.4
                    defaultValue: Math.sqrt(root.defaultBlackpoint*1000)
                    valueText: value*value/2
                    onValueChanged: {
                        filmProvider.blackpoint = value*value/1000
                        root.updateImage()
                    }
                    Connections {
                        target: root
                        onSetAllValues: {
                            blackpointSlider.value = Math.sqrt(blackpoint*1000)
                        }
                    }
                    Component.onCompleted: {
                        blackpointSlider.tooltipWanted.connect(root.tooltipWanted)
                    }
                }

                ToolSlider {
                    id: whitepointSlider
                    title: qsTr("White Clipping Point")
                    tooltipText: qsTr("The threshold for clipping the highlights. Vaguely analogous to adjusting exposure time in the darkroom.")
                    minimumValue: 0.1/1000
                    maximumValue: 2.5/1000
                    defaultValue: root.defaultWhitepoint
                    valueText: value*500// 1000/2
                    onValueChanged: {
                        filmProvider.whitepoint = value
                        root.updateImage()
                    }
                    Connections {
                        target: root
                        onSetAllValues: {
                            whitepointSlider.value = whitepoint
                        }
                    }
                    Component.onCompleted: {
                        whitepointSlider.tooltipWanted.connect(root.tooltipWanted)
                    }
                }


                ToolSlider {
                    id: shadowBrightnessSlider
                    title: qsTr("Shadow Brightness")
                    tooltipText: qsTr("The brightness of the generally darker regions.")
                    minimumValue: 0
                    maximumValue: 1
                    defaultValue: root.defaultShadowsY
                    valueText: value*1000
                    onValueChanged: {
                        filmProvider.shadowsY = value
                        root.updateImage()
                    }
                    Connections {
                        target: root
                        onSetAllValues: {
                            shadowBrightnessSlider.value = shadowsY
                        }
                    }
                    Component.onCompleted: {
                        shadowBrightnessSlider.tooltipWanted.connect(root.tooltipWanted)
                    }
                }

                ToolSlider {
                    id: highlightBrightnessSlider
                    title: qsTr("Highlight Brightness")
                    tooltipText: qsTr("The brightness of the generally lighter regions.")
                    minimumValue: 0
                    maximumValue: 1
                    defaultValue: root.defaultHighlightsY
                    valueText: value*1000
                    onValueChanged: {
                        filmProvider.highlightsY = value
                        root.updateImage()
                    }
                    Connections {
                        target: root
                        onSetAllValues: {
                            highlightBrightnessSlider.value = highlightsY
                        }
                    }
                    Component.onCompleted: {
                        highlightBrightnessSlider.tooltipWanted.connect(root.tooltipWanted)
                    }
                }

                ToolSlider {
                    id: vibranceSlider
                    title: qsTr("Vibrance")
                    tooltipText: qsTr("Adjust the vividness of the less-saturated colors in the image.")
                    minimumValue: -0.5
                    maximumValue: 0.5
                    defaultValue: root.defaultVibrance
                    valueText: value*200
                    onValueChanged: {
                        filmProvider.vibrance = value
                        root.updateImage()
                    }
                    Connections {
                        target: root
                        onSetAllValues: {
                            vibranceSlider.value = vibrance
                        }
                    }
                    Component.onCompleted: {
                        vibranceSlider.tooltipWanted.connect(root.tooltipWanted)
                    }
                }

                ToolSlider {
                    id: saturationSlider
                    title: qsTr("Saturation")
                    tooltipText: qsTr("Adjust the vividness of the entire image.")
                    minimumValue: -0.5
                    maximumValue: 0.5
                    defaultValue: root.defaultSaturation
                    valueText: value*200
                    onValueChanged: {
                        filmProvider.saturation = value
                        root.updateImage()
                    }
                    Connections {
                        target: root
                        onSetAllValues: {
                            saturationSlider.value = saturation
                        }
                    }
                    Component.onCompleted: {
                        saturationSlider.tooltipWanted.connect(root.tooltipWanted)
                    }
                }
            }
        }
        MouseArea {
            id: wheelstealer
            //This is to prevent scrolling from adjusting sliders.
            anchors.fill: toolList
            acceptedButtons: Qt.NoButton
            onWheel: {
                if (wheel.angleDelta.y > 0) {
                    //up
                    toolList.flick(0,600);
                }
                else {
                    //down
                    toolList.flick(0,-600);
                }
            }
        }
    }
}
