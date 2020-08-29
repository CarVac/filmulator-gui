import QtQuick 2.12
import QtQuick.Layouts 1.12
import Qt.labs.calendar 1.0
import "gui_components"
import "colors.js" as Colors

SlimSplitView {
    id: root
    Layout.fillWidth: true
    Layout.fillHeight: true
    orientation: Qt.Horizontal
    property real uiScale: 1

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
                    title: qsTr("Time zone")
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

                Rectangle {
                    id: calendarRect
                    color: Colors.darkGray
                    Layout.fillWidth: true
                    height: 250 * uiScale

                    property date selectedDate: settings.getOrganizeCaptureDate()
                    property int selectedDay: selectedDate.getDate()
                    property int selectedMonth: selectedDate.getMonth()
                    property int selectedYear: selectedDate.getFullYear()

                    property date calendarDate: settings.getOrganizeCaptureDate()
                    property int day: calendarDate.getDate()
                    property int month: calendarDate.getMonth()
                    property int year: calendarDate.getFullYear()

                    property bool notCompleted: true

                    Component.onCompleted: {
                        organizeModel.setMinMaxCaptureTime(settings.getOrganizeCaptureDate())
                        calendarRect.calendarDate = settings.getOrganizeCaptureDate()
                        calendarRect.selectedDate = settings.getOrganizeCaptureDate()
                        calendarRect.notCompleted = false
                    }

                    Connections {
                        target: organizeModel
                        function onCaptureDateChanged() {
                            if (!calendarRect.notCompleted) {
                                var newDate = organizeModel.getSelectedDate()
                                calendarRect.selectedDate = newDate
                                calendarRect.calendarDate = newDate
                                settings.organizeCaptureDate = newDate
                            }
                        }
                    }

                    //navigation
                    ToolButton {
                        id: backYear
                        text: "<<"
                        tooltipText: qsTr("Go to previous year")
                        width: 30 * uiScale
                        height: 30 * uiScale
                        x: 0 * uiScale
                        y: 0 * uiScale
                        onTriggered: {
                            calendarRect.calendarDate = new Date(calendarRect.year - 1, calendarRect.month)
                        }
                        Component.onCompleted: {
                            backYear.tooltipWanted.connect(root.tooltipWanted)
                        }
                        uiScale: root.uiScale
                    }
                    ToolButton {
                        id: backMonth
                        text: "<"
                        tooltipText: qsTr("Go to previous month")
                        width: 30 * uiScale
                        height: 30 * uiScale
                        x: 30 * uiScale
                        y: 0 * uiScale
                        onTriggered: {
                            var newMonth = calendarRect.month - 1
                            var newYear = calendarRect.year
                            if (newMonth < 0) {
                                newMonth = 11
                                newYear = newYear - 1
                            }
                            calendarRect.calendarDate = new Date(newYear, newMonth)
                        }
                        Component.onCompleted: {
                            backMonth.tooltipWanted.connect(root.tooltipWanted)
                        }
                        uiScale: root.uiScale
                    }
                    Text {
                        text: parent.year + "/" + (parent.month < 9 ? "0" : "") + (parent.month + 1)
                        width: parent.width - 120 * uiScale
                        height: 30 * uiScale
                        x: 60 * uiScale
                        y: 0 * uiScale
                        color: "white"
                        font.pixelSize: 12.0 * uiScale
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }
                    ToolButton {
                        id: forwardMonth
                        text: ">"
                        tooltipText: qsTr("Go to next month")
                        width: 30 * uiScale
                        height: 30 * uiScale
                        x: parent.width - 60 * uiScale
                        y: 0 * uiScale
                        onTriggered: {
                            var newMonth = calendarRect.month + 1
                            var newYear = calendarRect.year
                            if (newMonth > 11) {
                                newMonth = 0
                                newYear = newYear + 1
                            }
                            calendarRect.calendarDate = new Date(newYear, newMonth)
                        }
                        Component.onCompleted: {
                            forwardMonth.tooltipWanted.connect(root.tooltipWanted)
                        }
                        uiScale: root.uiScale
                    }
                    ToolButton {
                        id: forwardYear
                        text: ">>"
                        tooltipText: qsTr("Go to next year")
                        width: 30 * uiScale
                        height: 30 * uiScale
                        x: parent.width - 30 * uiScale
                        y: 0 * uiScale
                        onTriggered: {
                            calendarRect.calendarDate = new Date(calendarRect.year + 1, calendarRect.month)
                        }
                        Component.onCompleted: {
                            forwardYear.tooltipWanted.connect(root.tooltipWanted)
                        }
                        uiScale: root.uiScale
                    }

                    ColumnLayout {
                        y: 30 * uiScale
                        width: parent.width
                        height: 220 * uiScale
                        DayOfWeekRow {
                            locale: Qt.locale("en_US")
                            Layout.fillWidth: true
                            height: 24 * uiScale
                            delegate: Text {
                                text: model.shortName
                                color: "white"
                                font.pixelSize: 12.0 * uiScale
                                horizontalAlignment: Text.AlignHCenter
                                verticalAlignment: Text.AlignVCenter
                            }
                        }
                        MonthGrid {
                            locale: Qt.locale("en_US")
                            month: calendarRect.month
                            year: calendarRect.year
                            Layout.fillWidth: true
                            Layout.fillHeight: true
                            delegate: Text {
                                text: model.day
                                Layout.fillWidth: true
                                Layout.fillHeight: true
                                color: model.month === calendarRect.month ? "#FFFFFFFF" : "#88FFFFFF"
                                font.pixelSize: 12.0 * uiScale
                                horizontalAlignment: Text.AlignHCenter
                                verticalAlignment: Text.AlignVCenter
                                Rectangle {
                                    id: dayRectangle
                                    anchors.fill: parent
                                    z: -1
                                    color: Colors.darkGray
                                    border.color: (model.day === calendarRect.selectedDay && model.month === calendarRect.selectedMonth && model.year === calendarRect.selectedYear) ? Colors.medOrange : Colors.darkGray
                                    border.width: 2 * uiScale
                                    radius: 5 * uiScale
                                }
                                MouseArea {
                                    id: dayMouseArea
                                    anchors.fill: parent
                                    property int oldYear
                                    property int oldMonth
                                    property int oldDay
                                    onPressed: {
                                        filterListFlick.interactive = false
                                        //only respond to clicks in the current month
                                        oldYear = model.year
                                        oldMonth = model.month
                                        oldDay = model.day
                                        if (model.month === calendarRect.month) {
                                            var monthOne = model.month+1
                                            var monthStr = (monthOne < 10) ? ("0" + monthOne) : ("" + monthOne)
                                            var dayStr = (model.day < 10) ? ("0" + model.day) : ("" + model.day)
                                            var captureTimeString = model.year + "/" + monthStr + "/" + dayStr
                                            organizeModel.setMinMaxCaptureTimeString(captureTimeString)
                                            settings.organizeCaptureDate = new Date(model.year, model.month, model.day)
                                        }
                                    }
                                    onReleased: {
                                        var changed = (oldYear !== model.year) || (oldMonth !== model.month) || (oldDay !== model.day)
                                        if (model.month === calendarRect.month && changed) {
                                            var monthOne = model.month+1
                                            var monthStr = (monthOne < 10) ? ("0" + monthOne) : ("" + monthOne)
                                            var dayStr = (model.day < 10) ? ("0" + model.day) : ("" + model.day)
                                            var captureTimeString = model.year + "/" + monthStr + "/" + dayStr
                                            organizeModel.setMinMaxCaptureTimeString(captureTimeString)
                                            settings.organizeCaptureDate = new Date(model.year, model.month, model.day)
                                        }
                                        filterListFlick.interactive = true
                                    }
                                }
                            }
                        }
                    }
                }

                ToolSlider {
                    id: minRatingSlider
                    title: qsTr("Min Rating")
                    tooltipText: qsTr("Controls the minimum rating to display.")
                    minimumValue: -1
                    maximumValue: 5
                    stepSize: 1
                    tickmarksEnabled: true
                    value: settings.getOrganizeRating()
                    defaultValue: settings.getOrganizeRating()
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
                    title: qsTr("Min Rating")
                    tooltipText: qsTr("Controls the minimum rating to display.")
                    minimumValue: -1
                    maximumValue: 5
                    stepSize: 1
                    tickmarksEnabled: true
                    value: settings.getMaxOrganizeRating()
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
                anchors.fill: parent
                flow: GridView.FlowTopToBottom
                layoutDirection: Qt.LeftToRight
                cellWidth: 5 * uiScale
                cellHeight: dateHistogram.height

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
                    height: dateHistogram.height
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
                        tooltipText: qsTr('Date: ') + parent.theDate + '\n' + qsTr('Count: ') + parent.count
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
                anchors.fill: parent

                cellWidth: 320 * uiScale
                cellHeight: 320 * uiScale

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
}
