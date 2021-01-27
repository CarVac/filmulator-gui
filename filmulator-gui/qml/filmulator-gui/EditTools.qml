import QtQuick 2.12
import QtQuick.Layouts 1.12
import "gui_components"
import "generateHistogram.js" as Script
import "colors.js" as Colors

SlimSplitView {
    id: root
    property real uiScale: 1
    //width: 250
    anchors.margins: 3 * uiScale
    property real maxWidth: 500 * uiScale
    Layout.maximumWidth: maxWidth
    Layout.minimumWidth: 250 * uiScale
    orientation: Qt.Vertical

    property bool imageReady
    property bool cropping
    property bool imageError
    property bool onEditTab
    property string saveStatus: ""

    signal tooltipWanted(string text, int x, int y)

    Rectangle {
        width: parent.width
        Layout.minimumHeight: 50 * uiScale
        Layout.maximumHeight: 500 * uiScale
        height: 250 * uiScale
        color: Colors.lowGray
        Canvas {
            id: mainHistoCanvas
            anchors.fill: parent
            property real lineWidth: 2 * uiScale
            property real alpha: 1.0
            property real padding: 5 * uiScale
            canvasSize.width: root.maxWidth
            canvasSize.height: 500 * uiScale

            onWidthChanged: requestPaint()
            Connections {
                target: filmProvider
                function onHistFinalChanged() { mainHistoCanvas.requestPaint() }
            }

            onPaint: Script.generateHistogram(1,this.getContext('2d'),width,height,padding,lineWidth,root.uiScale)


            Component.onCompleted: mainHistoCanvas.requestPaint()
        }
    }

    Item {
        id: toolListItem
        width: parent.width
        Layout.fillHeight: true
        Flickable {
            id: toolList
            width: parent.width - 3 * uiScale
            height: parent.height
            flickableDirection: Qt.Vertical
            clip: true
            contentHeight: toolLayout.height

            boundsBehavior: Flickable.StopAtBounds
            flickDeceleration: 6000 * uiScale
            maximumFlickVelocity: 10000 * Math.sqrt(uiScale)

            onMovingChanged: { //reset params after mouse scrolling
                if (!moving) {
                    flickDeceleration = 6000 * uiScale
                    maximumFlickVelocity = 10000 * Math.sqrt(uiScale)
                }
            }

            ColumnLayout {
                id: toolLayout
                spacing: 0 * uiScale
                x: 3 * uiScale
                width: toolList.width - 6 * uiScale

                Rectangle {
                    id: topSpacer
                    color: "#00000000"//transparent
                    height: 3 * uiScale
                }

                Rectangle {
                    height: 30 * uiScale
                    Layout.fillWidth: true
                    color: Colors.lowGray

                    Canvas {
                        id: rawHistoCanvas
                        anchors.fill: parent

                        property real lineWidth: 2 * uiScale
                        property real alpha: 1.0
                        property real padding: 3 * uiScale

                        canvasSize.width: root.maxWidth
                        canvasSize.height: height

                        onWidthChanged: requestPaint();
                        Connections {
                            target: filmProvider
                            function onHistPreFilmChanged() { rawHistoCanvas.requestPaint() }
                        }

                        onPaint: Script.generateHistogram(4,this.getContext('2d'),width,height,padding,lineWidth,root.uiScale)

                        ToolTip {
                            id: rawHistoTooltip
                            tooltipText: qsTr("This is a histogram of the data in the raw file.")
                            Component.onCompleted: {
                                rawHistoTooltip.tooltipWanted.connect(root.tooltipWanted)
                            }
                        }
                    }
                }

                ToolSlider {
                    id: autoCASlider
                    title: qsTr("Auto CA Correction")
                    tooltipText: qsTr("Automatically correct directional color fringing. Use the lowest value needed because it can cause color shifts, but higher is stronger.\n\nNot available for non-Bayer photos.")
                    minimumValue: 0
                    maximumValue: 5
                    stepSize: 1
                    tickmarksEnabled: true
                    value: paramManager.caEnabled
                    defaultValue: paramManager.defCaEnabled
                    visible: paramManager.autoCaAvail
                    property bool bindingLoopCutoff: true
                    onValueChanged: {
                        if (!bindingLoopCutoff) {
                            paramManager.caEnabled = value
                            if (value > 0) {
                                lensfunCASwitch.setByAutoCA = true
                                lensfunCASwitch.isOn = false
                            }
                        }
                    }
                    onResetPerformed: paramManager.resetAutoCa()
                    onEditComplete: paramManager.writeback()
                    Connections {
                        target: paramManager
                        function onCaEnabledChanged() {
                            autoCASlider.value = paramManager.caEnabled
                        }
                        function onDefCaEnabledChanged() {
                            autoCASlider.defaultValue = paramManager.defCaEnabled
                        }
                    }
                    Component.onCompleted: {
                        autoCASlider.tooltipWanted.connect(root.tooltipWanted)
                        bindingLoopCutoff = false
                    }
                    uiScale: root.uiScale
                }

                ToolSwitch {
                    id: lensfunCASwitch
                    text: qsTr("Profiled CA")
                    tooltipText: qsTr("Correct directional color fringing based on a profile stored for this lens model.")
                    isOn: (paramManager.lensfunCa == 1)
                    defaultOn: (paramManager.defLensfunCa == 1)
                    visible: paramManager.lensfunCaAvail
                    property bool setByAutoCA: false
                    onIsOnChanged: {
                        paramManager.lensfunCa = isOn ? 1 : 0
                        if (isOn) {
                            autoCASlider.value = 0
                        }
                        if (!setByAutoCA) {
                            paramManager.writeback()
                        }
                    }
                    onResetToDefault: {
                        paramManager.lensfunCa = defaultOn ? 1 : 0
                        if (isOn) {
                            autoCASlider.value = 0
                        }
                        paramManager.resetLensfunCa()
                        paramManager.writeback()
                    }
                    Connections {
                        target: paramManager
                        function onLensfunCaChanged() {
                            lensfunCASwitch.isOn = (paramManager.lensfunCa == 1)
                        }
                        function onDefLensfunCaChanged() {
                            lensfunCASwitch.defaultOn = (paramManager.defLensfunCa == 1)
                        }
                    }
                    Component.onCompleted: {
                        lensfunCASwitch.tooltipWanted.connect(root.tooltipWanted)
                    }
                    uiScale: root.uiScale
                }

                ToolSwitch {
                    id: lensfunVignSwitch
                    text: qsTr("Profiled Vignetting")
                    tooltipText: qsTr("Correct vignetting based on a profile stored for this lens model.")
                    isOn: (paramManager.lensfunVign == 1)
                    defaultOn: (paramManager.defLensfunVign == 1)
                    visible: paramManager.lensfunVignAvail
                    onIsOnChanged: {
                        paramManager.lensfunVign = isOn ? 1 : 0
                        paramManager.writeback()
                    }
                    onResetToDefault: {
                        paramManager.lensfunVign = defaultOn ? 1 : 0
                        paramManager.resetLensfunVign()
                        paramManager.writeback()
                    }
                    Connections {
                        target: paramManager
                        function onLensfunVignChanged() {
                            lensfunVignSwitch.isOn = (paramManager.lensfunVign == 1)
                        }
                        function onDefLensfunVignChanged() {
                            lensfunVignSwitch.defaultOn = (paramManager.defLensfunVign == 1)
                        }
                    }
                    Component.onCompleted: {
                        lensfunVignSwitch.tooltipWanted.connect(root.tooltipWanted)
                    }
                    uiScale: root.uiScale
                }

                ToolSwitch {
                    id: lensfunDistSwitch
                    text: qsTr("Profiled Distortion")
                    tooltipText: qsTr("Correct geometric distortion based on a profile stored for this lens model.")
                    isOn: (paramManager.lensfunDist == 1)
                    defaultOn: (paramManager.defLensfunDist == 1)
                    visible: paramManager.lensfunDistAvail
                    onIsOnChanged: {
                        paramManager.lensfunDist = isOn ? 1 : 0
                        paramManager.writeback()
                    }
                    onResetToDefault: {
                        paramManager.lensfunDist = defaultOn ? 1 : 0
                        paramManager.resetLensfunDist()
                        paramManager.writeback()
                    }
                    Connections {
                        target: paramManager
                        function onLensfunDistChanged() {
                            lensfunDistSwitch.isOn = (paramManager.lensfunDist == 1)
                        }
                        function onDefLensfunDistChanged() {
                            lensfunDistSwitch.defaultOn = (paramManager.defLensfunDist == 1)
                        }
                    }
                    Component.onCompleted: {
                        lensfunDistSwitch.tooltipWanted.connect(root.tooltipWanted)
                    }
                    uiScale: root.uiScale
                }

                ToolSlider {
                    id: highlightRecoverySlider
                    title: qsTr("Highlight Recovery")
                    tooltipText: qsTr("Recover clipped highlights.\n\n0 clips after the preliminary white balance.\n1 is useful if 0 has restricted the red or blue channels in situations where no raw color channels are clipped.\n2 enables highlight reconstruction, which works best when only one channel is clipped, and when purple fringing isn't a problem.")
                    minimumValue: 0
                    maximumValue: 2
                    stepSize: 1
                    tickmarksEnabled: true
                    value: paramManager.highlights
                    defaultValue: paramManager.defHighlights
                    property bool bindingLoopCutoff: true
                    onValueChanged: {
                        if (!bindingLoopCutoff) {
                            paramManager.highlights = value
                        }
                    }
                    onEditComplete: paramManager.writeback()
                    Connections {
                        target: paramManager
                        function onHighlightsChanged() {
                            highlightRecoverySlider.value = paramManager.highlights
                        }
                        function onDefHighlightsChanged() {
                            highlightRecoverySlider.defaultValue = paramManager.defHighlights
                        }
                    }
                    Component.onCompleted: {
                        highlightRecoverySlider.tooltipWanted.connect(root.tooltipWanted)
                        bindingLoopCutoff = false
                    }
                    uiScale: root.uiScale
                }

                ToolSlider {
                    id: temperatureSlider
                    title: qsTr("Temperature")
                    tooltipText: qsTr("Correct the image color for a light source of the indicated Kelvin temperature.\n\nThe default value is the camera's chosen WB.")
                    minimumValue: Math.log(2000)
                    maximumValue: Math.log(20000)
                    value: Math.log(paramManager.temperature)
                    defaultValue: Math.log(paramManager.defTemperature)
                    valueText: Math.exp(value).toFixed(1)
                    onValueChanged: {
                        paramManager.temperature = Math.exp(value)
                    }
                    onEditComplete: paramManager.writeback()
                    Connections {
                        target: paramManager
                        function onTemperatureChanged() {
                            temperatureSlider.value = Math.log(paramManager.temperature)
                        }
                        function onDefTemperatureChanged() {
                            temperatureSlider.defaultValue = Math.log(paramManager.defTemperature)
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
                    tooltipText: qsTr("Correct for a green/magenta tinted light source. Larger values are greener, and smaller values are magenta.\n\nThe default value is the camera's chosen WB.")
                    minimumValue: Math.log(0.1)
                    maximumValue: Math.log(10)
                    value: Math.log(paramManager.tint)
                    defaultValue: Math.log(paramManager.defTint)
                    valueText: Math.exp(value).toFixed(4)
                    onValueChanged: {
                        paramManager.tint = Math.exp(value)
                    }
                    onEditComplete: paramManager.writeback()
                    Connections {
                        target: paramManager
                        function onTintChanged() {
                            tintSlider.value = Math.log(paramManager.tint)
                        }
                        function onDefTintChanged() {
                            tintSlider.defaultValue = Math.log(paramManager.defTint);
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
                    defaultValue: paramManager.defExposureComp
                    valueText: value.toFixed(4)
                    onValueChanged: {
                        paramManager.exposureComp = value
                    }
                    onEditComplete: paramManager.writeback()
                    Connections {
                        target: paramManager
                        function onExposureCompChanged() {
                            exposureCompSlider.value = paramManager.exposureComp
                        }
                        function onDefExposureCompChanged() {
                            exposureCompSlider.defaultValue = paramManager.defExposureComp
                        }
                    }
                    Component.onCompleted: {
                        exposureCompSlider.tooltipWanted.connect(root.tooltipWanted)
                    }
                    uiScale: root.uiScale
                }

                Rectangle {
                    height: 30 * uiScale
                    Layout.fillWidth: true
                    color: Colors.lowGray

                    Canvas {
                        id: preFilmHistoCanvas
                        anchors.fill: parent

                        property real lineWidth: 2 * uiScale
                        property real alpha: 1.0
                        property real padding: 3 * uiScale

                        canvasSize.width: root.maxWidth
                        canvasSize.height: height

                        onWidthChanged: requestPaint();
                        Connections {
                            target: filmProvider
                            function onHistPreFilmChanged() { preFilmHistoCanvas.requestPaint() }
                        }

                        onPaint: Script.generateHistogram(2,this.getContext('2d'),width,height,padding,lineWidth,root.uiScale)
                        Rectangle {
                            id: toeLine
                            height: parent.height
                            width: 1
                            color: toeSlider.pressed ? Colors.medOrange : "white"
                            x: parent.padding + paramManager.toeBoundary/65535*(parent.width-2*parent.padding)
                        }
                        Rectangle {
                            id: rolloffLine
                            height: parent.height
                            width: 1
                            color: rolloffSlider.pressed ? Colors.medOrange : "white"
                            x: parent.padding + paramManager.rolloffBoundary/65535*(parent.width-2*parent.padding)
                        }

                        ToolTip {
                            id: preFilmTooltip
                            tooltipText: qsTr("This is a histogram of the input to the film simulation.")
                            Component.onCompleted: {
                                preFilmTooltip.tooltipWanted.connect(root.tooltipWanted)
                            }
                        }
                    }
                }

                ToolSlider {
                    id: toeSlider
                    title: qsTr("Shadow Rolloff Point")
                    tooltipText: qsTr("This adjusts the contrast in the shadows of the image prior to the film simulation. Raising this darkens the image and makes it more contrasty.")
                    minimumValue: 0
                    maximumValue: Math.sqrt(10000)
                    value: Math.sqrt(paramManager.toeBoundary)
                    defaultValue: Math.sqrt(paramManager.defToeBoundary)
                    valueText: (value*value/65535).toFixed(6)
                    onValueChanged: {
                        paramManager.toeBoundary = value*value
                        //The parameter manager won't notify anything else that the param has changed, so we need to manually update the consumer
                        toeLine.x = Qt.binding(function() {return preFilmHistoCanvas.padding + paramManager.toeBoundary/65535*(preFilmHistoCanvas.width-2*preFilmHistoCanvas.padding)})
                    }
                    onEditComplete: paramManager.writeback()
                    Connections {
                        target: paramManager
                        function onToeBoundaryChanged() {
                            toeSlider.value = Math.sqrt(paramManager.toeBoundary)
                        }
                        function onDefToeBoundaryChanged() {
                            toeSlider.defaultValue = Math.sqrt(paramManager.defToeBoundary)
                        }
                    }
                    Component.onCompleted: {
                        toeSlider.tooltipWanted.connect(root.tooltipWanted)
                    }
                    uiScale: root.uiScale
                }

                ToolSlider {
                    id: rolloffSlider
                    title: qsTr("Highlight Rolloff Point")
                    tooltipText: qsTr("Sets the point above which the highlights gently stop getting brighter. This controls the saturation of the highlights, and only has a significant effect at high drama settings when the highlights get strongly darkened.\nIf you have a photo with no highlight clipping and none of it extends beyond the right of the prefilm histogram, feel free to raise this all the way to 1.\nIf you have highlight clipping and there are unpleasant color shifts, lower this to taste.")
                    minimumValue: 1
                    maximumValue: 65535
                    value: paramManager.rolloffBoundary
                    defaultValue: paramManager.defRolloffBoundary
                    valueText: (value/65535).toFixed(6)
                    onValueChanged: {
                        paramManager.rolloffBoundary = value
                        //The parameter manager won't notify anything else that the param has changed, so we need to manually update the consumer
                        rolloffLine.x = Qt.binding(function() {return preFilmHistoCanvas.padding + paramManager.rolloffBoundary/65535*(preFilmHistoCanvas.width-2*preFilmHistoCanvas.padding)})
                    }
                    onEditComplete: paramManager.writeback()
                    Connections {
                        target: paramManager
                        function onRolloffBoundaryChanged() {
                            rolloffSlider.value = paramManager.rolloffBoundary
                        }
                        function onDefRolloffBoundaryChanged() {
                            rolloffSlider.defaultValue = paramManager.defRolloffBoundary
                        }
                    }
                    Component.onCompleted: {
                        rolloffSlider.tooltipWanted.connect(root.tooltipWanted)
                    }
                    uiScale: root.uiScale
                }

                ToolSlider {
                    id: filmSizeSlider
                    title: qsTr("Film Area")
                    tooltipText: qsTr("Larger sizes emphasize smaller details and flatten contrast; smaller sizes emphasize larger regional contrasts. This has the same effect as film size in real film. If venturing into Medium or Large Format, keep the Drama slider below 40 to prevent overcooking.\nTypically, when adjusting this, change it until the point where small adjustments make noticeable changes in the appearance of the image. That's usually in the vicinity of the best setting.")
                    minimumValue: 1.2//less than log of 10
                    maximumValue: 6//greater than log of 300
                    value: Math.log(Math.sqrt(paramManager.filmArea))
                    defaultValue: Math.log(Math.sqrt(paramManager.defFilmArea))
                    //The following thresholds are 24mmx65mm and twice 6x9cm film's
                    // areas, respectively.
                    valueText: (Math.exp(value*2)).toFixed(1)//(Math.exp(value*2) < 1560) ? "SF" : (Math.exp(value*2) < 9408) ? "MF" : "LF"
                    onValueChanged: {
                        paramManager.filmArea = Math.exp(value*2)
                    }
                    onEditComplete: paramManager.writeback()
                    Connections {
                        target: paramManager
                        function onFilmAreaChanged() {
                            filmSizeSlider.value = Math.log(Math.sqrt(paramManager.filmArea))
                        }
                        function onDefFilmAreaChanged() {
                            filmSizeSlider.defaultValue = Math.log(Math.sqrt(paramManager.defFilmArea))
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
                    defaultValue: 100*paramManager.defLayerMixConst
                    valueText: value.toFixed(4)
                    onValueChanged: {
                        paramManager.layerMixConst = value/100;
                    }
                    onEditComplete: paramManager.writeback()
                    Connections {
                        target: paramManager
                        function onLayerMixConstChanged() {
                            filmDramaSlider.value = 100*paramManager.layerMixConst
                        }
                        function onDefLayerMixConstChanged() {
                            filmDramaSlider.defaultValue = 100*paramManager.defLayerMixConst
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
                    defaultOn: (paramManager.defAgitateCount == 0)
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
                        function onAgitateCountChanged() {
                            overdriveSwitch.isOn = (paramManager.agitateCount == 0)
                        }
                        function onDefAgitateCountChanged() {
                            overdriveSwitch.defaultOn = (paramManager.defAgitateCount == 0)
                        }
                    }
                    Component.onCompleted: {
                        overdriveSwitch.tooltipWanted.connect(root.tooltipWanted)
                    }
                    uiScale: root.uiScale
                }

                Rectangle {
                    height: 30 * uiScale
                    Layout.fillWidth: true
                    color: Colors.lowGray

                    Canvas {
                        id: postFilmHistoCanvas
                        anchors.fill: parent

                        property real lineWidth: 2 * uiScale
                        property real alpha: 1.0
                        property real padding: 3 * uiScale

                        canvasSize.width: root.maxWidth
                        canvasSize.height: height

                        onWidthChanged: requestPaint();
                        Connections {
                            target: filmProvider
                            function onHistPostFilmChanged() { postFilmHistoCanvas.requestPaint() }
                        }

                        onPaint: Script.generateHistogram(3,this.getContext('2d'),width,height,padding,lineWidth,root.uiScale)
                        Rectangle {
                            id: blackpointLine
                            height: parent.height
                            width: 1
                            color: blackpointSlider.pressed ? Colors.medOrange : "white"
                            x: parent.padding + paramManager.blackpoint/.0025*(parent.width-2*parent.padding)
                            //The .0025 is the highest bin in the post filmulator histogram.
                        }

                        Rectangle {
                            id: whitepointLine
                            height: parent.height
                            width: 1
                            color: whitepointSlider.pressed ? Colors.medOrange : (blackpointSlider.pressed ? Colors.brighterOrange : "white")
                            x: parent.padding + (paramManager.blackpoint+paramManager.whitepoint)/.0025*(parent.width-2*parent.padding)
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
                }

                ToolSlider {
                    id: blackpointSlider
                    title: qsTr("Black Clipping Point")
                    tooltipText: qsTr("This controls the threshold for crushing the shadows. You can see its position in the post-film histogram.")
                    minimumValue: 0
                    maximumValue: 1.4
                    value: Math.sqrt(paramManager.blackpoint*1000)
                    defaultValue: Math.sqrt(paramManager.defBlackpoint*1000)
                    valueText: (value*value/2).toFixed(5)
                    onValueChanged: {
                        paramManager.blackpoint = value*value/1000
                        //The parameter manager won't notify anything else that the param has changed, so we need to manually update the consumers
                        blackpointLine.x = Qt.binding(function() {return postFilmHistoCanvas.padding + paramManager.blackpoint/.0025*(postFilmHistoCanvas.width-2*postFilmHistoCanvas.padding)})
                        whitepointLine.x = Qt.binding(function() {return postFilmHistoCanvas.padding + (paramManager.blackpoint+paramManager.whitepoint)/.0025*(postFilmHistoCanvas.width-2*postFilmHistoCanvas.padding)})
                    }
                    onEditComplete: paramManager.writeback()
                    Connections {
                        target: paramManager
                        function onBlackpointChanged() {
                            blackpointSlider.value = Math.sqrt(paramManager.blackpoint*1000)
                        }
                        function onDefBlackpointChanged() {
                            blackpointSlider.defaultValue = Math.sqrt(paramManager.defBlackpoint*1000)
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
                    defaultValue: paramManager.defWhitepoint
                    valueText: (value*500).toFixed(5)// 1000/2
                    onValueChanged: {
                        paramManager.whitepoint = value
                        //The parameter manager won't notify anything else that the param has changed, so we need to manually update the consumer
                        whitepointLine.x = Qt.binding(function() {return postFilmHistoCanvas.padding + (paramManager.blackpoint+paramManager.whitepoint)/.0025*(postFilmHistoCanvas.width-2*postFilmHistoCanvas.padding)})
                    }
                    onEditComplete: paramManager.writeback()
                    Connections {
                        target: paramManager
                        function onWhitepointChanged() {
                            whitepointSlider.value = paramManager.whitepoint
                        }
                        function onDefWhitepointChanged() {
                            whitepointSlider.defaultValue = paramManager.defWhitepoint
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
                    defaultValue: paramManager.defShadowsY
                    valueText: (value*1000).toFixed(3)
                    onValueChanged: {
                        paramManager.shadowsY = value
                    }
                    onEditComplete: paramManager.writeback()
                    Connections {
                        target: paramManager
                        function onShadowsYChanged() {
                            shadowBrightnessSlider.value = paramManager.shadowsY
                        }
                        function onDefShadowsYChanged() {
                            shadowBrightnessSlider.defaultValue = paramManager.defShadowsY
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
                    defaultValue: paramManager.defHighlightsY
                    valueText: (value*1000).toFixed(3)
                    onValueChanged: {
                        paramManager.highlightsY = value
                    }
                    onEditComplete: paramManager.writeback()
                    Connections {
                        target: paramManager
                        function onHighlightsYChanged() {
                            highlightBrightnessSlider.value = paramManager.highlightsY
                        }
                        function onDefHighlightsYChanged() {
                            highlightBrightnessSlider.defaultValue = paramManager.defHighlightsY
                        }
                    }
                    Component.onCompleted: {
                        highlightBrightnessSlider.tooltipWanted.connect(root.tooltipWanted)
                    }
                    uiScale: root.uiScale
                }

                ToolSwitch {
                    id: monochromeSwitch
                    text: qsTr("Monochrome")
                    tooltipText: qsTr("Turn this on to convert to black-and-white.")
                    isOn: paramManager.monochrome
                    defaultOn: paramManager.defMonochrome
                    onIsOnChanged: {
                        paramManager.monochrome = isOn
                        paramManager.writeback()
                    }
                    onResetToDefault: {
                        paramManager.monochrome = isOn
                        paramManager.writeback()
                    }
                    Connections {
                        target: paramManager
                        function onMonochromeChanged() {
                            monochromeSwitch.isOn = paramManager.monochrome
                        }
                        function onDefMonochromeChanged() {
                            monochromeSwitch.defaultOn = paramManager.defMonochrome
                        }
                    }
                    Component.onCompleted: {
                        monochromeSwitch.tooltipWanted.connect(root.tooltipWanted)
                    }
                    uiScale: root.uiScale
                }

                ToolSlider {
                    id: vibranceSlider
                    visible: !monochromeSwitch.isOn
                    highlight: monochromeSwitch.hovered
                    title: qsTr("Vibrance")
                    tooltipText: qsTr("This adjusts the vividness of the less-saturated colors in the image.")
                    minimumValue: -0.5
                    maximumValue: 0.5
                    value: paramManager.vibrance
                    defaultValue: paramManager.defVibrance
                    valueText: (value*200).toFixed(3)
                    onValueChanged: {
                        paramManager.vibrance = value
                    }
                    onEditComplete: paramManager.writeback()
                    Connections {
                        target: paramManager
                        function onVibranceChanged() {
                            vibranceSlider.value = paramManager.vibrance
                        }
                        function onDefVibranceChanged() {
                            vibranceSlider.defaultValue = paramManager.defVibrance
                        }
                    }
                    Component.onCompleted: {
                        vibranceSlider.tooltipWanted.connect(root.tooltipWanted)
                    }
                    uiScale: root.uiScale
                }

                ToolSlider {
                    id: saturationSlider
                    visible: !monochromeSwitch.isOn
                    highlight: monochromeSwitch.hovered
                    title: qsTr("Saturation")
                    tooltipText: qsTr("This adjusts the vividness of the entire image.")
                    minimumValue: -0.5
                    maximumValue: 0.5
                    value: paramManager.saturation
                    defaultValue: paramManager.defSaturation
                    valueText: (value*200).toFixed(3)
                    onValueChanged: {
                        paramManager.saturation = value
                    }
                    onEditComplete: paramManager.writeback()
                    Connections {
                        target: paramManager
                        function onSaturationChanged() {
                            saturationSlider.value = paramManager.saturation
                        }
                        function onDefSaturationChanged() {
                            saturationSlider.defaultValue = paramManager.defSaturation
                        }
                    }
                    Component.onCompleted: {
                        saturationSlider.tooltipWanted.connect(root.tooltipWanted)
                    }
                    uiScale: root.uiScale
                }

                ToolSlider {
                    id: bwRmultSlider
                    visible: monochromeSwitch.isOn
                    highlight: monochromeSwitch.hovered
                    title: qsTr("Red Weight")
                    tooltipText: qsTr("How much to weight the red channel when converting to monochrome.")
                    minimumValue: -2.0
                    maximumValue: 2.0
                    value: paramManager.bwRmult
                    defaultValue: paramManager.defBwRmult
                    onValueChanged: {
                        paramManager.bwRmult = value
                    }
                    onEditComplete: paramManager.writeback()
                    Connections {
                        target: paramManager
                        function onBwRmultChanged() {
                            bwRmultSlider.value = paramManager.bwRmult
                        }
                        function onDefBwRmultChanged() {
                            bwRmultSlider.defaultValue = paramManager.defBwRmult
                        }
                    }
                    Component.onCompleted: {
                        bwRmultSlider.tooltipWanted.connect(root.tooltipWanted)
                    }
                    uiScale: root.uiScale
                }

                ToolSlider {
                    id: bwGmultSlider
                    visible: monochromeSwitch.isOn
                    highlight: monochromeSwitch.hovered
                    title: qsTr("Green Weight")
                    tooltipText: qsTr("How much to weight the green channel when converting to monochrome.")
                    minimumValue: -2.0
                    maximumValue: 2.0
                    value: paramManager.bwGmult
                    defaultValue: paramManager.defBwGmult
                    onValueChanged: {
                        paramManager.bwGmult = value
                    }
                    onEditComplete: paramManager.writeback()
                    Connections {
                        target: paramManager
                        function onBwGmultChanged() {
                            bwGmultSlider.value = paramManager.bwGmult
                        }
                        function onDefBwGmultChanged() {
                            bwGmultSlider.defaultValue = paramManager.defBwGmult
                        }
                    }
                    Component.onCompleted: {
                        bwGmultSlider.tooltipWanted.connect(root.tooltipWanted)
                    }
                    uiScale: root.uiScale
                }

                ToolSlider {
                    id: bwBmultSlider
                    visible: monochromeSwitch.isOn
                    highlight: monochromeSwitch.hovered
                    title: qsTr("Blue Weight")
                    tooltipText: qsTr("How much to weight the blue channel when converting to monochrome.")
                    minimumValue: -2.0
                    maximumValue: 2.0
                    value: paramManager.bwBmult
                    defaultValue: paramManager.defBwBmult
                    onValueChanged: {
                        paramManager.bwBmult = value
                    }
                    onEditComplete: paramManager.writeback()
                    Connections {
                        target: paramManager
                        function onBwBmultChanged() {
                            bwBmultSlider.value = paramManager.bwBmult
                        }
                        function onDefBwBmultChanged() {
                            bwBmultSlider.defaultValue = paramManager.defBwBmult
                        }
                    }
                    Component.onCompleted: {
                        bwBmultSlider.tooltipWanted.connect(root.tooltipWanted)
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

        Item {
            id: scrollbarHolderToolList
            x: parent.width - 10*uiScale
            y: 0
            width: 10*uiScale
            height: parent.height

            Rectangle {
                id: scrollbarBackgroundToolList
                color: Colors.darkGray
                opacity: 0

                x: parent.width-width - 1*uiScale
                width: 3 * uiScale

                y: 0
                height: parent.height

                transitions: Transition {
                    NumberAnimation {
                        property: "width"
                        duration: 200
                    }
                    NumberAnimation {
                        property: "opacity"
                        duration: 200
                    }
                }
                states: State {
                    name: "hovered"
                    when: scrollbarMouseAreaToolList.containsMouse || scrollbarMouseAreaToolList.pressed
                    PropertyChanges {
                        target: scrollbarBackgroundToolList
                        width: 8 * uiScale
                        opacity: 0.5
                    }
                }
            }

            Rectangle {
                id: scrollbarToolList
                color: scrollbarMouseAreaToolList.pressed ? Colors.medOrange : scrollbarMouseAreaToolList.containsMouse ? Colors.weakOrange : Colors.middleGray
                radius: 1.5*uiScale

                x: parent.width-width - 1 * uiScale
                width: 3 * uiScale

                y: 1 * uiScale + (0.99*toolList.visibleArea.yPosition) * (parent.height - 2*uiScale)
                height: (0.99*toolList.visibleArea.heightRatio + 0.01) * (parent.height - 2*uiScale)

                transitions: Transition {
                    NumberAnimation {
                        property: "width"
                        duration: 200
                    }
                }
                states: State {
                    name: "hovered"
                    when: scrollbarMouseAreaToolList.containsMouse || scrollbarMouseAreaToolList.pressed
                    PropertyChanges {
                        target: scrollbarToolList
                        width: 8 * uiScale
                    }
                }
            }
            MouseArea {
                id: scrollbarMouseAreaToolList
                anchors.fill: parent
                hoverEnabled: true
                acceptedButtons: Qt.LeftButton
                onWheel: {
                    //See the Queue.qml file for the math behind this.

                    //We have to duplicate the wheelstealer one because this has higher priority for some reason.
                    //Set the scroll deceleration and max speed higher for wheel scrolling.
                    //It should be reset when the view stops moving.
                    //For now, this is 10x higher than standard.
                    var deceleration = 6000 * 10
                    toolList.flickDeceleration = deceleration * uiScale
                    toolList.maximumFlickVelocity = 10000 * Math.sqrt(uiScale*10)

                    var velocity = toolList.verticalVelocity/uiScale
                    var newVelocity = velocity

                    var distance = 100
                    if (wheel.angleDelta.y > 0 && !toolList.atXBeginning && !root.dragging) {
                        //Leftward; up on the scroll wheel.
                        newVelocity = uiScale*(velocity <= 0 ? Math.sqrt((velocity*velocity/(4*deceleration) + distance*wheel.angleDelta.y/(120))*4*deceleration) : 0)
                        newVelocity = Math.min(newVelocity, toolList.maximumFlickVelocity)
                        toolList.flick(0,1)
                        toolList.flick(0, newVelocity)
                    } else if (wheel.angleDelta.y < 0 && !toolList.atXEnd && !root.dragging) {
                        //Rightward; down on the scroll wheel.
                        newVelocity = uiScale*(velocity >= 0 ? Math.sqrt((velocity*velocity/(4*deceleration) + distance*wheel.angleDelta.y/(-120))*4*deceleration) : 0)
                        newVelocity = -Math.min(newVelocity, toolList.maximumFlickVelocity)
                        toolList.flick(0,-1)
                        toolList.flick(0, newVelocity)
                    }
                }

                property bool overDragThresh: false
                property real pressY
                property real viewY
                onPositionChanged: {
                    if (pressed) {
                        var deltaY = mouse.y - pressY
                        var scrollHeight = scrollbarMouseAreaToolList.height - scrollbarToolList.height - 2*uiScale
                        var relativeDelta = deltaY / scrollHeight
                        var scrollMargin = toolList.contentHeight - toolList.height
                        toolList.contentY = Math.max(0, Math.min(scrollMargin, viewY + relativeDelta * scrollMargin))
                    }
                }

                onPressed: {
                    preventStealing = true
                    toolList.cancelFlick()
                    pressY = mouse.y
                    viewY = toolList.contentY
                }
                onReleased: {
                    preventStealing = false
                }
            }
        }

        MouseArea {
            id: wheelstealer
            //Custom scrolling implementation because the default flickable one sucks.
            anchors.fill: parent
            acceptedButtons: Qt.NoButton
            onWheel: {
                //See the Queue.qml file for the math behind this.

                //Set the scroll deceleration and max speed higher for wheel scrolling.
                //It should be reset when the view stops moving.
                //For now, this is 10x higher than standard.
                var deceleration = 6000 * 2
                toolList.flickDeceleration = deceleration * uiScale
                toolList.maximumFlickVelocity = 10000 * Math.sqrt(uiScale*2)

                var velocity = toolList.verticalVelocity/uiScale
                var newVelocity = velocity

                var distance = 30 //the tool list is relatively short so it needs less scrolling
                if (wheel.angleDelta.y > 0 && !toolList.atYBeginning) {
                    //up on the scroll wheel.
                    newVelocity = uiScale*(velocity <= 0 ? Math.sqrt((velocity*velocity/(4*deceleration) + distance*wheel.angleDelta.y/(120))*4*deceleration) : 0)
                    newVelocity = Math.min(newVelocity, toolList.maximumFlickVelocity)
                    toolList.flick(0,1)
                    toolList.flick(0, newVelocity)
                } else if (wheel.angleDelta.y < 0 && !toolList.atYEnd) {
                    //down on the scroll wheel.
                    newVelocity = uiScale*(velocity >= 0 ? Math.sqrt((velocity*velocity/(4*deceleration) + distance*wheel.angleDelta.y/(-120))*4*deceleration) : 0)
                    newVelocity = -Math.min(newVelocity, toolList.maximumFlickVelocity)
                    toolList.flick(0,-1)
                    toolList.flick(0, newVelocity)
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
            notDisabled: root.imageReady && (!root.cropping) && (!root.imageError)
            text: qsTr("Save TIFF")
            tooltipText: root.cropping ? qsTr("Finish cropping to save the result.") : qsTr("Save a TIFF to the directory containing the raw file.")
            onTriggered: {
                filmProvider.writeTiff()
                queueModel.markSaved(paramManager.imageIndex)
                root.savestatus = "saved"
            }
            Component.onCompleted: {
                saveTIFFButton.tooltipWanted.connect(root.tooltipWanted)
            }
        }
        ToolButton {
            id: saveJPEGButton
            width: parent.width/2
            height: 40 * uiScale
            uiScale: root.uiScale
            x: width
            y: 0
            notDisabled: root.imageReady && (!root.cropping) && (!root.imageError)
            text: qsTr("Save JPEG")
            tooltipText: root.cropping ? qsTr("Finish cropping to save the result.") : qsTr("Save a JPEG to the directory containing the raw file.")
            onTriggered: {
                filmProvider.writeJpeg()
                queueModel.markSaved(paramManager.imageIndex)
                root.saveStatus = "saved"
            }

            Shortcut {
                sequence: StandardKey.Save
                onActivated: {
                    if (saveJPEGButton.notDisabled && root.onEditTab) {
                        filmProvider.writeJpeg()
                        queueModel.markSaved(paramManager.imageIndex)
                        root.saveStatus = "saved"
                    }
                }
            }

            Component.onCompleted: {
                saveJPEGButton.tooltipWanted.connect(root.tooltipWanted)
            }
        }
    }
    Connections {
        target: paramManager
        function onFileError() {
            root.imageError = true
        }
        function onFilenameChanged() {
            root.imageError = false
        }
    }
}
