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
                    defaultValue: settings.getOrganizeTZ()
                    onValueChanged: {
                        organizeModel.timeZone = value
                        settings.organizeTZ = value
                        organizeModel.setOrganizeQuery()
                    }
                    Component.onCompleted: {
                        timezoneOffset.tooltipWanted.connect(root.tooltipWanted)
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
                            organizeModel.minCaptureTime = tempDate
                            organizeModel.maxCaptureTime = tempDate
                            organizeModel.setOrganizeQuery()
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
                            organizeModel.minCaptureTime = tempDate
                            organizeModel.maxCaptureTime = tempDate
                            organizeModel.setOrganizeQuery()
                        }
                        selectedDate = tempDate
                        filterListFlick.interactive = true
                    }

                    uiScale: root.uiScale
                    Component.onCompleted: {
                        captureCalendar.tooltipWanted.connect(root.tooltipWanted)
                    }
                }

                ToolSlider {
                    id: ratingSlider
                    title: qsTr("Rating")
                    tooltipText: qsTr("Controls the minimum rating to display.")
                    minimumValue: 0
                    maximumValue: 5
                    stepSize: 1
                    defaultValue: settings.getOrganizeRating()
                    onValueChanged: {
                        settings.organizeRating = value
                        organizeModel.minRating = value
                        organizeModel.setOrganizeQuery()
                    }
                    uiScale: root.uiScale
                    Component.onCompleted: {
                        ratingSlider.tooltipWanted.connect(root.tooltipWanted)
                    }
                }
            }
        }
    }

    Rectangle {
        id: gridViewBox
        color: "#202020"
        Layout.fillWidth: true

        GridView {
            id: gridView
            anchors.fill: parent

            cellWidth: 320 * uiScale
            cellHeight: 320 * uiScale

            boundsBehavior: Flickable.StopAtBounds
            maximumFlickVelocity: 50000 * uiScale

            delegate: OrganizeDelegate {
                id: organizeDelegate
                rootDir: organizeModel.thumbDir()

                searchID: STsearchID
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

            Component.onCompleted: {
/*                organizeModel.minCaptureTime = 0
                organizeModel.maxCaptureTime = 1400000000
                organizeModel.minImportTime = 0
                organizeModel.maxImportTime = 1400000000
                organizeModel.minProcessedTime = 0
                organizeModel.maxProcessedTime = 1400000000*/
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
