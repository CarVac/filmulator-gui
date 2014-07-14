import QtQuick 2.2
import QtQuick.Controls 1.2
import QtQuick.Controls.Styles 1.2
import QtQuick.Layouts 1.1
import "gui_components"

SplitView {
    id: root
    anchors.fill: parent
    orientation: Qt.Horizontal

    Rectangle {
        id: filterList
        color: "#202020"
        width: 250
        Layout.maximumWidth: 500
        ColumnLayout {
            id: filterLayout

            ToolSlider {
                id: timezoneOffset
                width: filterList.width
                title: qsTr("TimezoneSplit")
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

            delegate: OrganizeDelegate {
                rootDir: organizeModel.thumbDir()

                searchID: STsearchID

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
    }
}
