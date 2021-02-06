import QtQuick 2.12
import QtQuick.Layouts 1.12
import "gui_components"
import "colors.js" as Colors

SlimSplitView {
    id: root
    Layout.fillWidth: true
    Layout.fillHeight: true
    orientation: Qt.Horizontal
    property real uiScale: 1

    property int itemCount: organizeModel.imageCount

    property bool onOrganizeTab

    signal tooltipWanted(string text, int x, int y)

    Rectangle {
        id: filterList
        color: "#202020"
        width: 250 * uiScale
        Layout.maximumWidth: 500 * uiScale
        Layout.minimumWidth: 200 * uiScale
        Flickable {
            id: filterListFlick
            width: parent.width
            height: parent.height
            flickableDirection: Qt.Vertical
            clip: true
            contentHeight: filterLayout.height
            boundsBehavior: Flickable.StopAtBounds
            //interactive: false

            ColumnLayout {
                id: filterLayout
                x: 3 * uiScale
                width: parent.width - 6 * uiScale

                Rectangle {
                    id: topSpacer
                    color: "#00000000"
                    height: 3 * uiScale
                }

                ToolSlider {
                    id: timezoneOffset
                    title: qsTr("Time Zone")
                    tooltipText: qsTr("Controls when the day is divided for the date filters.")
                    minimumValue: -14
                    maximumValue: 14
                    stepSize: 1
                    tickmarksEnabled: true
                    tickmarkFactor: 6
                    tickmarkOffset: 2
                    minorTicksEnabled: true
                    value: settings.getOrganizeTZ()
                    defaultValue: settings.getOrganizeTZ()
                    onValueChanged: {
                        settings.organizeTZ = value
                        organizeModel.timeZone = value
                        gridView.returnToBounds()
                    }
                    Component.onCompleted: {
                        timezoneOffset.tooltipWanted.connect(root.tooltipWanted)
                        organizeModel.timeZone = value
                        gridView.returnToBounds()
                    }
                    uiScale: root.uiScale
                }

                ToolSlider {
                    id: minRatingSlider
                    title: qsTr("Min Rating")
                    tooltipText: qsTr("Controls the minimum rating of images to display.")
                    minimumValue: -1
                    maximumValue: 5
                    stepSize: 1
                    tickmarksEnabled: true
                    value: settings.getOrganizeRating()
                    defaultValue: settings.getOrganizeRating()
                    valueText: value < 0 ? "X" : value
                    onValueChanged: {
                        settings.organizeRating = value
                        organizeModel.minRating = value
                        gridView.returnToBounds()
                    }
                    onEditComplete: {
                        if (minRatingSlider.value > maxRatingSlider.value) {
                            maxRatingSlider.value = minRatingSlider.value
                            settings.maxOrganizeRating = value
                            organizeModel.maxRating = value
                            gridView.returnToBounds()
                        }
                    }

                    uiScale: root.uiScale
                    Component.onCompleted: {
                        minRatingSlider.tooltipWanted.connect(root.tooltipWanted)
                        organizeModel.minRating = value
                    }
                }

                ToolSlider {
                    id: maxRatingSlider
                    title: qsTr("Max Rating")
                    tooltipText: qsTr("Controls the maximum rating of images to display.")
                    minimumValue: -1
                    maximumValue: 5
                    stepSize: 1
                    tickmarksEnabled: true
                    value: settings.getMaxOrganizeRating()
                    valueText: value < 0 ? "X" : value
                    defaultValue: settings.getMaxOrganizeRating()
                    onValueChanged: {
                        settings.maxOrganizeRating = value
                        organizeModel.maxRating = value
                        gridView.returnToBounds()
                    }
                    onEditComplete: {
                        if (minRatingSlider.value > maxRatingSlider.value) {
                            minRatingSlider.value = maxRatingSlider.value
                            settings.organizeRating = value
                            organizeModel.minRating = value
                            gridView.returnToBounds()
                        }
                    }
                    uiScale: root.uiScale
                    Component.onCompleted: {
                        maxRatingSlider.tooltipWanted.connect(root.tooltipWanted)
                        organizeModel.maxRating = value
                    }
                }

                ToolButton {
                    id: enqueueAll
                    text: qsTr("Enqueue All")
                    tooltipText: qsTr("Puts every visible image in the work queue. If any is already there, it is left where it was.")
                    Layout.fillWidth: true
                    onTriggered: queueModel.batchEnqueue(organizeModel.adaptableModelQuery(true))
                    Component.onCompleted: {
                        enqueueAll.tooltipWanted.connect(root.tooltipWanted)
                    }
                    uiScale: root.uiScale
                }
            }
        }
    }

    SlimSplitView {
        id: dateHistoSplit
        orientation: Qt.Vertical
        Layout.fillWidth: true
        Rectangle {
            id: dateHistogram
            color: "#202020"
            height: 100 * uiScale
            Layout.maximumHeight: 300 * uiScale
            Layout.minimumHeight: 30 * uiScale

            GridView {
                id: dateHistoView
                x: 0
                y: 0
                width: parent.width
                height: parent.height
                flow: GridView.FlowTopToBottom
                layoutDirection: Qt.LeftToRight
                cellWidth: 5 * uiScale
                cellHeight: height

                property real trueContentWidth: Math.max(width, cellWidth*dateHistoModel.dateHistoSize)

                boundsBehavior: Flickable.StopAtBounds
                flickDeceleration: 6000 * uiScale
                maximumFlickVelocity: 10000 * Math.sqrt(uiScale)

                onMovingChanged: { //reset params after mouse scrolling
                    if (!moving) {
                        flickDeceleration = 6000 * uiScale
                        maximumFlickVelocity = 10000 * Math.sqrt(uiScale)
                    }
                }

                delegate: Rectangle {
                    id: dateHistoDelegate
                    width: 5.01 * uiScale //This has to be sliiightly bigger to ensure overlap
                    property int julianDay: julday
                    property string theDate: thedate
                    property int count: thecount
                    property string yearMonthString: yearmonth
                    property int month: themonth
                    property int day: theday
                    property real contentAmount: Math.min(1, (count > 0) ? (Math.log(count)+1)/16 : 0)
                    property bool sel: organizeModel.isDateSelected(theDate)//selectedDate == theDate
                    height: dateHistoView.height
                    color: (1===themonth%2) ? (sel ? Colors.darkOrangeH : Colors.darkGrayH) : (sel ? Colors.darkOrangeL : Colors.darkGrayL)
                    clip: true
                    Text {
                        id: monthYearLabel
                        color: "white"
                        width: parent.width
                        height: 12 * uiScale
                        x: 5 * uiScale - parent.width * (parent.day - 1)
                        y: 3 * uiScale
                        font.pixelSize: 12 * uiScale
                        text: parent.yearMonthString
                    }

                    Rectangle {
                        id: dateHistoRectangle
                        width: parent.width
                        y: parent.height*(1-contentAmount)
                        height: parent.height*contentAmount
                        color: parent.sel ? Colors.brightOrange : Colors.lightOrange
                    }

                    ToolTip {
                        id: dateHistoTooltip
                        anchors.fill: parent
                        tooltipText: qsTr('Date: ') + parent.theDate + '\n' + qsTr('Photos: ') + parent.count
                        milliSecondDelay: 0
                        Component.onCompleted: {
                            dateHistoTooltip.tooltipWanted.connect(root.tooltipWanted)
                        }
                    }
                    MouseArea {//Change the date when double-clicked.
                        id: dateChanger
                        anchors.fill: parent
                        acceptedButtons: Qt.LeftButton | Qt.RightButton
                        onDoubleClicked: {
                            if(mouse.button === Qt.LeftButton) {
                                organizeModel.setMinMaxCaptureTimeString(parent.theDate)
                                gridView.returnToBounds()
                            }
                        }
                        onClicked: {
                            if(mouse.button === Qt.RightButton || (mouse.button === Qt.LeftButton && (mouse.modifiers & Qt.ShiftModifier))) {
                                organizeModel.extendMinMaxCaptureTimeString(parent.theDate)
                                gridView.returnToBounds()
                            }
                        }
                    }
                    Connections {
                        target: organizeModel
                        function onCaptureDateChanged() {
                            sel = organizeModel.isDateSelected(theDate)
                        }
                    }
                }

                Connections {
                    target: dateHistoModel
                    function onDateHistoModelChanged() {
                        var xPos = dateHistoView.contentX
                        organizeModel.setDateHistoQuery()//yes it's controlled by organizemodel
                        dateHistoView.contentX = xPos
                    }
                }
                Connections {
                    target: importModel
                    function onSearchTableChanged() {
                        var xPos = dateHistoView.contentX
                        organizeModel.setDateHistoQuery()//yes it's controlled by organizemodel
                        dateHistoView.contentX = xPos

                    }
                }
                Connections {
                    target: queueModel
                    function onSearchTableChanged() {
                        var xPos = dateHistoView.contentX
                        organizeModel.setDateHistoQuery()//yes it's controlled by organizemodel
                        dateHistoView.contentX = xPos

                    }
                }

                Component.onCompleted: {
                    organizeModel.setDateHistoQuery()
                    dateHistoView.model = dateHistoModel
                    positionViewAtEnd()
                }
            }

            Item {
                id: scrollbarHolderDateHisto
                x: 0
                y: 0
                width: parent.width
                height: 15*uiScale

                Rectangle {
                    id: scrollbarDateHisto
                    color: scrollbarMouseAreaDateHisto.pressed ? Colors.medOrange : scrollbarMouseAreaDateHisto.containsMouse ? Colors.weakOrange : Colors.middleGray
                    opacity: scrollbarMouseAreaDateHisto.containsMouse || scrollbarMouseAreaDateHisto.pressed ? 0.65 : 1
                    radius: 1.5*uiScale

                    y: 1 * uiScale
                    height: 3 * uiScale

                    x: 1 * uiScale + (0.99*dateHistoView.visibleArea.xPosition) * (parent.width - 2*uiScale)
                    width: (0.99*dateHistoView.visibleArea.widthRatio + 0.01) * (parent.width - 2*uiScale)

                    transitions: Transition {
                        NumberAnimation {
                            property: "height"
                            duration: 200
                        }
                    }
                    states: State {
                        name: "hovered"
                        when: scrollbarMouseAreaDateHisto.containsMouse || scrollbarMouseAreaDateHisto.pressed
                        PropertyChanges {
                            target: scrollbarDateHisto
                            height: 12 * uiScale
                        }
                    }
                }
                MouseArea {
                    id: scrollbarMouseAreaDateHisto
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
                        dateHistoView.flickDeceleration = deceleration * uiScale
                        dateHistoView.maximumFlickVelocity = 10000 * Math.sqrt(uiScale*10)

                        var velocity = dateHistoView.horizontalVelocity/uiScale
                        var newVelocity = velocity

                        var distance = 100
                        if (wheel.angleDelta.y > 0 && !dateHistoView.atXBeginning && !root.dragging) {
                            //Leftward; up on the scroll wheel.
                            newVelocity = uiScale*(velocity <= 0 ? Math.sqrt((velocity*velocity/(4*deceleration) + distance*wheel.angleDelta.y/(120))*4*deceleration) : 0)
                            newVelocity = Math.min(newVelocity, dateHistoView.maximumFlickVelocity)
                            dateHistoView.flick(1,0)
                            dateHistoView.flick(newVelocity, 0)
                        } else if (wheel.angleDelta.y < 0 && !dateHistoView.atXEnd && !root.dragging) {
                            //Rightward; down on the scroll wheel.
                            newVelocity = uiScale*(velocity >= 0 ? Math.sqrt((velocity*velocity/(4*deceleration) + distance*wheel.angleDelta.y/(-120))*4*deceleration) : 0)
                            newVelocity = -Math.min(newVelocity, dateHistoView.maximumFlickVelocity)
                            dateHistoView.flick(-1,0)
                            dateHistoView.flick(newVelocity, 0)
                        }
                    }

                    property bool overDragThresh: false
                    property real pressX
                    property real viewX
                    onPositionChanged: {
                        if (pressed) {
                            var deltaX = mouse.x - pressX
                            var scrollWidth = scrollbarMouseAreaDateHisto.width - scrollbarDateHisto.width - 2*uiScale
                            var relativeDelta = deltaX / scrollWidth
                            var scrollMargin = dateHistoView.trueContentWidth - dateHistoView.width
                            dateHistoView.contentX = Math.max(0, Math.min(scrollMargin, viewX + relativeDelta * scrollMargin))
                        }
                    }

                    onPressed: {
                        preventStealing = true
                        dateHistoView.cancelFlick()
                        pressX = mouse.x
                        viewX = dateHistoView.contentX
                    }
                    onReleased: {
                        preventStealing = false
                    }
                }
            }

            MouseArea {
                id: wheelstealerDateHisto
                anchors.fill: parent
                acceptedButtons: Qt.NoButton
                onWheel: {
                    //See the Queue.qml file for the math behind this.

                    //Set the scroll deceleration and max speed higher for wheel scrolling.
                    //It should be reset when the view stops moving.
                    //For now, this is 10x higher than standard.
                    var deceleration = 6000 * 10
                    dateHistoView.flickDeceleration = deceleration * uiScale
                    dateHistoView.maximumFlickVelocity = 10000 * Math.sqrt(uiScale*10)

                    var velocity = dateHistoView.horizontalVelocity/uiScale
                    var newVelocity = velocity

                    var distance = 100
                    if (wheel.angleDelta.y > 0 && !dateHistoView.atXBeginning && !root.dragging) {
                        //Leftward; up on the scroll wheel.
                        newVelocity = uiScale*(velocity <= 0 ? Math.sqrt((velocity*velocity/(4*deceleration) + distance*wheel.angleDelta.y/(120))*4*deceleration) : 0)
                        newVelocity = Math.min(newVelocity, dateHistoView.maximumFlickVelocity)
                        dateHistoView.flick(1,0)
                        dateHistoView.flick(newVelocity, 0)
                    } else if (wheel.angleDelta.y < 0 && !dateHistoView.atXEnd && !root.dragging) {
                        //Rightward; down on the scroll wheel.
                        newVelocity = uiScale*(velocity >= 0 ? Math.sqrt((velocity*velocity/(4*deceleration) + distance*wheel.angleDelta.y/(-120))*4*deceleration) : 0)
                        newVelocity = -Math.min(newVelocity, dateHistoView.maximumFlickVelocity)
                        dateHistoView.flick(-1,0)
                        dateHistoView.flick(newVelocity, 0)
                    }
                }
            }
        }

        Rectangle {
            id: gridViewBox
            color: "#202020"
            Layout.fillHeight: true

            GridView {
                id: gridView
                x: 0
                y: 0
                width: parent.width - 5*uiScale
                height: parent.height

                cellWidth: 320 * uiScale
                cellHeight: 320 * uiScale

                property int colCount: Math.floor(width/cellWidth)
                property int rowCount: Math.ceil(organizeModel.imageCount/colCount)

                property real trueContentHeight: Math.max(height, cellHeight*rowCount)

                clip: true

                boundsBehavior: Flickable.StopAtBounds
                flickDeceleration: 6000 * uiScale
                maximumFlickVelocity: 10000 * Math.sqrt(uiScale)

                onMovingChanged: { //reset params after mouse scrolling
                    if (!moving) {
                        flickDeceleration = 6000 * uiScale
                        maximumFlickVelocity = 10000 * Math.sqrt(uiScale)
                    }
                }

                delegate: OrganizeDelegate {
                    id: organizeDelegate
                    rootDir: organizeModel.thumbDir()

                    searchID: STsearchID
                    captureTime: STcaptureTime
                    importTime: STimportTime
                    lastProcessedTime: STlastProcessedTime
                    rating: STrating
                    filename: STfilename
                    thumbWritten: STthumbWritten

                    isCurrentItem: index === gridView.currentIndex

                    //Toggles selection
                    onSelectImage: gridView.currentIndex = (isCurrentItem ? -1 : index)

                    //Enqueues the image when double clicked
                    onEnqueueImage: queueModel.enQueue(STsearchID)

                    //Writes the rating back to the database.
                    onRate: organizeModel.setRating(STsearchID, ratingIn)

                    uiScale: root.uiScale
                    Component.onCompleted: organizeDelegate.tooltipWanted.connect(root.tooltipWanted)
                }

                Connections {
                    target: importModel
                    function onSearchTableChanged() {
                        var yPos = gridView.contentY
                        organizeModel.setOrganizeQuery()
                        gridView.contentY = yPos
                    }
                }
                Connections {
                    target: queueModel
                    function onSearchTableChanged() {
                        var yPos = gridView.contentY
                        organizeModel.setOrganizeQuery()
                        gridView.contentY = yPos
                    }
                }

                Connections {
                    target: organizeModel
                    function onOrganizeFilterChanged() {
                        var yPos = gridView.contentY
                        organizeModel.setOrganizeQuery()
                        gridView.contentY = yPos
                    }
                }

                Component.onCompleted: {
                    organizeModel.minRating = 0
                    organizeModel.maxRating = 5
                    organizeModel.setOrganizeQuery()
                    gridView.model = organizeModel
                }
            }

            Item {
                id: scrollbarHolderGridView
                x: parent.width - 15*uiScale
                y: 0
                width: 15*uiScale
                height: parent.height

                Rectangle {
                    id: scrollbarBackgroundGridView
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
                        when: scrollbarMouseAreaGridView.containsMouse || scrollbarMouseAreaGridView.pressed
                        PropertyChanges {
                            target: scrollbarBackgroundGridView
                            width: 12 * uiScale
                            opacity: 0.5
                        }
                    }
                }

                Rectangle {
                    id: scrollbarGridView
                    color: scrollbarMouseAreaGridView.pressed ? Colors.medOrange : scrollbarMouseAreaGridView.containsMouse ? Colors.weakOrange : Colors.middleGray
                    radius: 1.5*uiScale

                    x: parent.width-width - 1 * uiScale
                    width: 3 * uiScale

                    y: 1 * uiScale + (0.99*gridView.visibleArea.yPosition) * (parent.height - 2*uiScale)
                    height: (0.99*gridView.visibleArea.heightRatio + 0.01) * (parent.height - 2*uiScale)

                    transitions: Transition {
                        NumberAnimation {
                            property: "width"
                            duration: 200
                        }
                    }
                    states: State {
                        name: "hovered"
                        when: scrollbarMouseAreaGridView.containsMouse || scrollbarMouseAreaGridView.pressed
                        PropertyChanges {
                            target: scrollbarGridView
                            width: 12 * uiScale
                        }
                    }
                }
                MouseArea {
                    id: scrollbarMouseAreaGridView
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
                        gridView.flickDeceleration = deceleration * uiScale
                        gridView.maximumFlickVelocity = 10000 * Math.sqrt(uiScale*10)

                        var velocity = gridView.verticalVelocity/uiScale
                        var newVelocity = velocity

                        var distance = 100
                        if (wheel.angleDelta.y > 0 && !gridView.atXBeginning && !root.dragging) {
                            //Leftward; up on the scroll wheel.
                            newVelocity = uiScale*(velocity <= 0 ? Math.sqrt((velocity*velocity/(4*deceleration) + distance*wheel.angleDelta.y/(120))*4*deceleration) : 0)
                            newVelocity = Math.min(newVelocity, gridView.maximumFlickVelocity)
                            gridView.flick(0,1)
                            gridView.flick(0, newVelocity)
                        } else if (wheel.angleDelta.y < 0 && !gridView.atXEnd && !root.dragging) {
                            //Rightward; down on the scroll wheel.
                            newVelocity = uiScale*(velocity >= 0 ? Math.sqrt((velocity*velocity/(4*deceleration) + distance*wheel.angleDelta.y/(-120))*4*deceleration) : 0)
                            newVelocity = -Math.min(newVelocity, gridView.maximumFlickVelocity)
                            gridView.flick(0,-1)
                            gridView.flick(0, newVelocity)
                        }
                    }

                    property bool overDragThresh: false
                    property real pressY
                    property real viewY
                    onPositionChanged: {
                        if (pressed) {
                            var deltaY = mouse.y - pressY
                            var scrollHeight = scrollbarMouseAreaGridView.height - scrollbarGridView.height - 2*uiScale
                            var relativeDelta = deltaY / scrollHeight
                            var scrollMargin = gridView.trueContentHeight - gridView.height
                            gridView.contentY = Math.max(0, Math.min(scrollMargin, viewY + relativeDelta * scrollMargin))
                        }
                    }

                    onPressed: {
                        preventStealing = true
                        gridView.cancelFlick()
                        pressY = mouse.y
                        viewY = gridView.contentY
                    }
                    onReleased: {
                        preventStealing = false
                    }
                }
            }

            Rectangle {
                id: noImageTopBox
                width: 400 * uiScale
                height: noImageTopText.contentHeight + 30 * uiScale
                x: (parent.width-width)/2
                y: 20 * uiScale
                color: Colors.darkGray
                border.color: Colors.lowGray
                border.width: 2 * uiScale
                radius: 10 * uiScale
                visible: root.itemCount < 1

                Text {
                    id: noImageTopText
                    width: 375 * uiScale
                    anchors.centerIn: parent
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                    color: "white"
                    font.pixelSize: 14.0 * uiScale
                    wrapMode: Text.Wrap
                    text: qsTr("Double-click on the Date Histogram above to view photos from a given day. Shift-click or right-click to set a date range. Press right/left to switch days, and shift to select multiple days.")
                }
            }

            MouseArea {
                id: wheelstealer
                //Custom scrolling implementation because the default flickable one sucks.
                anchors.fill: gridView
                acceptedButtons: Qt.NoButton
                onWheel: {
                    //See the Queue.qml file for the math behind this.

                    //Set the scroll deceleration and max speed higher for wheel scrolling.
                    //It should be reset when the view stops moving.
                    //For now, this is 10x higher than standard.
                    var deceleration = 6000 * 10
                    gridView.flickDeceleration = deceleration * uiScale
                    gridView.maximumFlickVelocity = 10000 * Math.sqrt(uiScale*10)

                    var velocity = gridView.verticalVelocity/uiScale
                    var newVelocity = velocity

                    var distance = 100
                    if (wheel.angleDelta.y > 0 && !gridView.atYBeginning) {
                        //up on the scroll wheel.
                        newVelocity = uiScale*(velocity <= 0 ? Math.sqrt((velocity*velocity/(4*deceleration) + distance*wheel.angleDelta.y/(120))*4*deceleration) : 0)
                        newVelocity = Math.min(newVelocity, gridView.maximumFlickVelocity)
                        gridView.flick(0,1)
                        gridView.flick(0, newVelocity)
                    } else if (wheel.angleDelta.y < 0 && !gridView.atYEnd) {
                        //down on the scroll wheel.
                        newVelocity = uiScale*(velocity >= 0 ? Math.sqrt((velocity*velocity/(4*deceleration) + distance*wheel.angleDelta.y/(-120))*4*deceleration) : 0)
                        newVelocity = -Math.min(newVelocity, gridView.maximumFlickVelocity)
                        gridView.flick(0,-1)
                        gridView.flick(0, newVelocity)
                    }
                }
            }
        }
    }
    Shortcut {
        id: prevDay
        sequence: StandardKey.MoveToPreviousChar
        enabled: onOrganizeTab
        onActivated: {
            organizeModel.incrementCaptureTime(-1)
        }
    }
    Shortcut {
        id: nextDay
        sequence: StandardKey.MoveToNextChar
        enabled: onOrganizeTab
        onActivated: {
            organizeModel.incrementCaptureTime(1)
        }
    }
    Shortcut {
        id: prevDayRange
        sequence: StandardKey.SelectPreviousChar
        enabled: onOrganizeTab
        onActivated: {
            organizeModel.extendCaptureTimeRange(-1)
        }
    }
    Shortcut {
        id: nextDayRange
        sequence: StandardKey.SelectNextChar
        enabled: onOrganizeTab
        onActivated: {
            organizeModel.extendCaptureTimeRange(1)
        }
    }
}
