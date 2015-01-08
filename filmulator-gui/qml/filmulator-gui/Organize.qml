import QtQuick 2.2
import QtQuick.Controls 1.2
import QtQuick.Controls.Styles 1.2
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
                    onClicked: {
                        settings.organizeCaptureDate = selectedDate
                        organizeModel.minCaptureTime = selectedDate
                        organizeModel.maxCaptureTime = selectedDate
                        organizeModel.setOrganizeQuery()
                    }
                    uiScale: root.uiScale
                    Component.onCompleted: {
                        captureCalendar.tooltipWanted.connect(root.tooltipWanted)
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
                rootDir: organizeModel.thumbDir()

                searchID: STsearchID
                importTime: STimportTime
                lastProcessedTime: STlastProcessedTime

                uiScale: root.uiScale

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
