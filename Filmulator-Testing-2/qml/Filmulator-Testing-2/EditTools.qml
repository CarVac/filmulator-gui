import QtQuick 2.2
import QtQuick.Controls 1.1
import QtQuick.Layouts 1.1
import "gui_components"
import "generateHistogram.js" as Script

Rectangle {
    id: root
    color: "#404040"
    width: 250
    Layout.maximumWidth: 500
    Layout.minimumWidth: 250

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

    signal setAllValues()

    signal updateImage()

    SplitView{
        anchors.fill: parent
        orientation: Qt.Vertical

        Canvas {
            id:mainHistoCanvas
            width:parent.width
            Layout.minimumHeight: 50
            height:250
            property int lineWidth: 1
            property real alpha: 1.0
//            property int hist: filmProvider.histFinal
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
                contentWidth: contentItem.childrenRect.width
                contentHeight: contentItem.childrenRect.height
                ColumnLayout {
                    spacing: 0
                    width: toolListItem.width
/*                    CheckBox{
                        id: defaultToneCurveCheckBox
                        text: qsTr("Default Tone Curve")
                        checked: defaultCurve
                        onClicked: {
                            filmProvider.defaultToneCurveEnabled = checked
                            root.updateImage()
                        }
                        Connections {
                            target: root
                            onSetAllValues: {
                                defaultToneCurveCheckBox.checked = defaultCurve
                            }
                        }
                    }*/

                    ToolSlider {
                        id: exposureCompSlider
                        title: qsTr("Exposure Compensation")
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
                    }

                    ToolSlider {
                        id: highlightRecoverySlider
                        title: qsTr("Highlight Recovery")
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
                    }

                    Canvas {
                        id: preFilmHistoCanvas
                        Layout.fillWidth: true
                        //It seems that since this is in a layout, you can't bind dimensions or locations.
                        // Makes sense, given that the layout is supposed to abstract that away.
                        height: 30
                        property int lineWidth: 1
                        property real alpha: 1.0
//                        property int hist: filmProvider.histPreFilm
                        property int padding: 3
                        antialiasing: true

                        onWidthChanged: requestPaint();
                        Connections {
                            target: filmProvider
                            onHistPreFilmChanged: preFilmHistoCanvas.requestPaint()
                        }

                        onPaint: Script.generateHistogram(2,this.getContext('2d'),width,height,padding,lineWidth)
                   }

                    ToolSlider {
                        id: filmSizeSlider
                        title: qsTr("Film Area")
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
                    }

                    ToolSlider {
                        id: filmPowerSlider
                        title: qsTr("Drama")
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
                                filmSizeSlider.value = 100*layerMixConst
                            }
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
//                        property int hist: filmProvider.histPostFilm
                        property int padding: 3
                        antialiasing: true

                        onWidthChanged: requestPaint();
                        Connections {
                            target: filmProvider
                            onHistPostFilmChanged: postFilmHistoCanvas.requestPaint()
                        }

                        onPaint: Script.generateHistogram(3,this.getContext('2d'),width,height,padding,lineWidth)
                    }

                    ToolSlider {
                        id: blackpointSlider
                        title: qsTr("Black Clipping Point")
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
                    }

                    ToolSlider {
                        id: whitepointSlider
                        title: qsTr("White Clipping Point")
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
                    }


                    ToolSlider {
                        id: shadowBrightnessSlider
                        title: qsTr("Shadow Brightness")
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
                    }

                    ToolSlider {
                        id: highlightBrightnessSlider
                        title: qsTr("Highlight Brightness")
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
}
