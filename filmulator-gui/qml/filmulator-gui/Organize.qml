import QtQuick 2.3
import QtQuick.Controls 1.3
import QtQuick.Controls.Styles 1.3
import QtQuick.Layouts 1.1
import "gui_components"
import "colors.js" as Colors

SplitView {
    id: root
    anchors.fill: parent
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
                    value: settings.getOrganizeTZ()
                    defaultValue: settings.getOrganizeTZ()
                    onValueChanged: {
                        console.log("timezone slider changed")
                        settings.organizeTZ = value
                        organizeModel.timeZone = value
                    }
                    Component.onCompleted: {
                        timezoneOffset.tooltipWanted.connect(root.tooltipWanted)
                        organizeModel.timeZone = value
                    }
                    uiScale: root.uiScale
                }

                ToolCalendar {
                    id: captureCalendar
                    minimumDate: "1970-01-01"
                    maximumDate: "2038-01-01"
                    selectedDate: settings.getOrganizeCaptureDate()
                    onPressed: {
                        filterListFlick.interactive = false
                        var isChanged = 0
                        if (!initialClick && !secondClick) {
                            initialClick = true
                            if (tempDate.toString() !== date.toString()) {
                                isChanged = 1
                            }
                            if (tempDate.toDateString().slice(4,7) !== date.toDateString().slice(4,7)) {
                                //If the month changed, we don't want to update on any further movements.
                                //We start a counter to ignore x frames of movement.
                                monthChanged = 15
                            }
                            tempDate = date
                        } else {
                            initialClick = false
                            secondClick = true
                            if (monthChanged <= 0) {
                                // If the ignore counter is empty
                                if (tempDate.toString() !== date.toString()) {
                                    isChanged = 1
                                }
                                tempDate = date
                            } else {
                                //If we still have to see
                                monthChanged = monthChanged - 1;
                            }
                        }
                        if (isChanged === 1) {
                            organizeModel.setMinMaxCaptureTime(tempDate)
                        }
                    }

                    onReleased: {
                        var isChanged = 0
                        //If the mouse has moved, and to a new date, and in the same month
                        if (!initialClick && tempDate.toString() !== date.toString() && monthChanged <= 0) {
                            //Use the new position.
                            tempDate = date
                            //Then do have it update the model.
                            isChanged = 1
                        }

                        initialClick = false
                        secondClick = false
                        monthChanged = 0
                        settings.organizeCaptureDate = tempDate
                        if (isChanged === 1) {
                            organizeModel.setMinMaxCaptureTime(tempDate)
                        }
                        selectedDate = tempDate
                        filterListFlick.interactive = true
                    }

                    uiScale: root.uiScale
                    Component.onCompleted: {
                        captureCalendar.tooltipWanted.connect(root.tooltipWanted)
                        organizeModel.setMinMaxCaptureTime(selectedDate)
                    }
                }

                ToolSlider {
                    id: ratingSlider
                    title: qsTr("Rating")
                    tooltipText: qsTr("Controls the minimum rating to display.")
                    minimumValue: 0
                    maximumValue: 5
                    stepSize: 1
                    value: settings.getOrganizeRating()
                    defaultValue: settings.getOrganizeRating()
                    onValueChanged: {
                        settings.organizeRating = value
                        organizeModel.minRating = value
                    }
                    uiScale: root.uiScale
                    Component.onCompleted: {
                        ratingSlider.tooltipWanted.connect(root.tooltipWanted)
                        organizeModel.minRating = value
                    }
                }
            }
        }
    }

    SplitView {
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

                delegate: Rectangle {
                    id: dateHistoDelegate
                    width: 5.01 * uiScale //This has to be sliiightly bigger to ensure overlap
                    property string date: thedate
                    property int count: thecount
                    property int month: themonth
                    property string yearMonthString: yearmonth
                    property int day: theday
                    property real contentAmount: Math.min(1, (count > 0) ? (Math.log(count)+1)/16 : 0)
                    height: dateHistogram.height
                    color: (1===themonth%2) ? Colors.darkGrayH : Colors.darkGrayL
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
                        color: Colors.lightOrange
                    }

                    ToolTip {
                        id: dateHistoTooltip
                        anchors.fill: parent
                        tooltipText:  qsTr('Date: ') + parent.date + '\n' + qsTr('Count: ') + parent.count
                        Component.onCompleted: {
                            dateHistoTooltip.tooltipWanted.connect(root.tooltipWanted)
                        }
                    }
                }

                Connections {
                    target: dateHistoModel
                    onBasicSqlModelChanged: {
                        var xPos = dateHistoView.contentX
                        organizeModel.setDateHistoQuery()//yes it's controlled by organizemodel
                        dateHistoView.contentX = xPos
                    }
                }
                Connections {
                    target: importModel
                    onSearchTableChanged: {
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

                boundsBehavior: Flickable.StopAtBounds
                maximumFlickVelocity: 50000 * uiScale
                clip: true

                delegate: OrganizeDelegate {
                    id: organizeDelegate
                    rootDir: organizeModel.thumbDir()

                    searchID: STsearchID
                    captureTime: STcaptureTime
                    importTime: STimportTime
                    lastProcessedTime: STlastProcessedTime
                    rating: STrating
                    filename: STfilename

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
                    onSearchTableChanged: {
                        var yPos = gridView.contentY
                        organizeModel.setOrganizeQuery()
                        gridView.contentY = yPos
                    }
                }

                Connections {
                    target: organizeModel
                    onOrganizeFilterChanged: {
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
                    var velocity = gridView.verticalVelocity
                    if (wheel.angleDelta.y > 0 && !gridView.atYBeginning) {
                        //up
                        //This formula makes each click of the wheel advance the 'target' a fixed distance.
                        gridView.flick(0, velocity < 0 ? Math.sqrt(velocity*velocity + 2000000) : (velocity == 0 ? 500 : 0))
                        //It's not 1,000,000 (1000 squared) because it feels slightly sluggish at that level.
                        //And 1000 isn't higher because otherwise a single scroll click is too far.
                    }
                    else if (wheel.angleDelta.y < 0 && !gridView.atYEnd) {
                        //down
                        gridView.flick(0, velocity > 0 ? -Math.sqrt(velocity*velocity + 2000000) : (velocity == 0 ? -500 : 0))
                    }
                }
            }
        }
    }
}
