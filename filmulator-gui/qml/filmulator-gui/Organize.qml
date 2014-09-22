import QtQuick 2.2
import QtQuick.Controls 1.2
import QtQuick.Controls.Styles 1.2
import QtQuick.Layouts 1.1
import "gui_components"

SplitView {
    id: root
    anchors.fill: parent
    orientation: Qt.Horizontal

    signal tooltipWanted(string text, int x, int y)

    Rectangle {
        id: filterList
        color: "#202020"
        width: 250
        Layout.maximumWidth: 500
        Layout.minimumWidth: 200
        ColumnLayout {
            id: filterLayout

            ToolSlider {
                id: timezoneOffset
                width: filterList.width
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
            }

            Calendar {
                id: captureCalendar
                width: filterList.width
                minimumDate: "1970-01-01"
                maximumDate: "2038-01-01"
                selectedDate: settings.getOrganizeCaptureDate()
                onClicked: {
                    settings.organizeCaptureDate = selectedDate
                    organizeModel.minCaptureTime = selectedDate
                    organizeModel.maxCaptureTime = selectedDate
                    organizeModel.setOrganizeQuery()
                }
                //Todo: style this.
                style: CalendarStyle {
                    gridVisible: false
                    background: Rectangle {
                        color: "#303030"
                        implicitWidth: 250
                        implicitHeight: 250
                    }

                    dayDelegate: Rectangle {
                        color: "#303030"
                        border.color: styleData.selected ? "#FF8800" : "#303030"
                        border.width: 2
                        radius: 5
                        Label {
                            text: styleData.date.getDate()
                            anchors.centerIn: parent
                            color: styleData.visibleMonth && styleData.valid ? "#FFFFFF" : "#888888"
                        }
                    }
                    navigationBar: Rectangle {
                        id: calendarNavBar
                        color: "#303030"
                        height: 30
                        ToolButton {
                            id: backYear
                            text: "<<"
                            tooltipText: qsTr("Previous year")
                            width: 30
                            height: 30
                            x: 0
                            y: 0
                            action: Action {
                                onTriggered: {
                                    control.showPreviousYear()
                                }
                            }
                            Component.onCompleted: {
                                backYear.tooltipWanted.connect(root.tooltipWanted)
                            }
                        }
                        ToolButton {
                            id: backMonth
                            text: "<"
                            tooltipText: qsTr("Previous month")
                            width: 30
                            height: 30
                            x: 30
                            y: 0
                            action: Action {
                                onTriggered: {
                                    control.showPreviousMonth()
                                }
                            }
                             Component.onCompleted: {
                                backMonth.tooltipWanted.connect(root.tooltipWanted)
                            }
                        }
                        Label {
                            text: (control.visibleMonth + 1) + "/" + control.visibleYear
                            anchors.centerIn: parent
                            color: "white"
                        }
                        ToolButton {
                            id: forwardMonth
                            text: ">"
                            tooltipText: qsTr("Next month")
                            width: 30
                            height: 30
                            x: parent.width-60
                            y: 0
                            action: Action {
                                onTriggered: {
                                    control.showNextMonth()
                                }
                            }
                              Component.onCompleted: {
                                forwardMonth.tooltipWanted.connect(root.tooltipWanted)
                            }
                       }
                        ToolButton {
                            id: forwardYear
                            text: ">>"
                            tooltipText: qsTr("Next year")
                            width: 30
                            height: 30
                            x: parent.width-30
                            y: 0
                            action: Action {
                                onTriggered: {
                                    control.showNextYear()
                                }
                            }
                             Component.onCompleted: {
                                forwardYear.tooltipWanted.connect(root.tooltipWanted)
                            }
                        }
                    }
                    dayOfWeekDelegate: Rectangle {
                        color: "#303030"
                        implicitHeight: 30
                        Label {
                            text: control.__locale.dayName(styleData.dayOfWeek,control.dayOfWeekFormat)
                            color: "white"
                            anchors.centerIn: parent
                        }
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

            cellWidth: 320
            cellHeight: 320

            boundsBehavior: Flickable.StopAtBounds
            maximumFlickVelocity: 50000

            delegate: OrganizeDelegate {
                rootDir: organizeModel.thumbDir()

                searchID: STsearchID
                importTime: STimportTime
                lastProcessedTime: STlastProcessedTime

                MouseArea {
                    anchors.fill: parent
                    onClicked: parent.GridView.view.currentIndex = index
                    onDoubleClicked: {
                        queueModel.enQueue(STsearchID)
                    }
                }
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
                    gridView.flick(0, velocity < 0 ? Math.sqrt(velocity*velocity + 2000000) : 1000)
                    //It's not 1,000,000 (1000 squared) because it feels slightly sluggish at that level.
                    //And 1000 isn't higher because otherwise a single scroll click is too far.
                }
                else if (wheel.angleDelta.y < 0 && !gridView.atYEnd) {
                    //down
                    gridView.flick(0, velocity > 0 ? -Math.sqrt(velocity*velocity + 2000000) : -1000)
                }
            }
        }
    }
}
