import QtQuick 2.3
import QtQuick.Controls 1.2
import QtQuick.Controls.Styles 1.2
import "../colors.js" as Colors
import "."

Calendar {
    id: root
    property real uiScale: 1
    implicitHeight: 250 * uiScale
    implicitWidth: parent.width
    minimumDate: "1970-01-01"
    maximumDate: "2038-01-01"
    property date tempDate
    property int monthChanged: 0
    property bool initialClick: false
    property bool secondClick: false

    signal tooltipWanted(string text, int x, int y)

    style: CalendarStyle {
        gridVisible: false
        background: Rectangle {
            color: Colors.darkGray
            implicitWidth: 250 * uiScale
            implicitHeight: 250 * uiScale
        }

        dayDelegate: Rectangle {
            color: Colors.darkGray
            border.color: styleData.selected ? Colors.medOrange : Colors.darkGray
            border.width: 2 * uiScale
            radius: 5 * uiScale
            Text {
                text: styleData.date.getDate()
                anchors.centerIn: parent
                color: styleData.visibleMonth && styleData.valid ? "white" : Colors.middleGray
                font.pixelSize: 12.0 * uiScale
            }
        }
        navigationBar: Rectangle {
            id: calendarNavBar
            color: Colors.darkGray
            height: 30 * uiScale
            ToolButton {
                id: backYear
                text: "<<"
                tooltipText: qsTr("Previous year")
                width: 30 * uiScale
                height: 30 * uiScale
                x: 0 * uiScale
                y: 0 * uiScale
                onTriggered: {
                    control.showPreviousYear()
                }
                Component.onCompleted: {
                    backYear.tooltipWanted.connect(root.tooltipWanted)
                }
                uiScale: root.uiScale
            }
            ToolButton {
                id: backMonth
                text: "<"
                tooltipText: qsTr("Previous month")
                width: 30 * uiScale
                height: 30 * uiScale
                x: 30 * uiScale
                y: 0 * uiScale
                onTriggered: {
                    control.showPreviousMonth()
                }
                Component.onCompleted: {
                    backMonth.tooltipWanted.connect(root.tooltipWanted)
                }
                uiScale: root.uiScale
            }
            Text {
                text: control.visibleYear + "/" + (control.visibleMonth < 9 ? "0" : "" ) + (control.visibleMonth + 1)
                anchors.centerIn: parent
                color: "white"
                font.pixelSize: 12.0 * uiScale
            }
            ToolButton {
                id: forwardMonth
                text: ">"
                tooltipText: qsTr("Next month")
                width: 30 * uiScale
                height: 30 * uiScale
                x: parent.width - 60 * uiScale
                y: 0 * uiScale
                onTriggered: {
                    control.showNextMonth()
                }
                Component.onCompleted: {
                    forwardMonth.tooltipWanted.connect(root.tooltipWanted)
                }
                uiScale: root.uiScale
           }
            ToolButton {
                id: forwardYear
                text: ">>"
                tooltipText: qsTr("Next year")
                width: 30 * uiScale
                height: 30 * uiScale
                x: parent.width - 30 * uiScale
                y: 0 * uiScale
                onTriggered: {
                    control.showNextYear()
                }
                Component.onCompleted: {
                    forwardYear.tooltipWanted.connect(root.tooltipWanted)
                }
                uiScale: root.uiScale
            }
        }
        dayOfWeekDelegate: Rectangle {
            color: Colors.darkGray
            implicitHeight: 30 * uiScale
            Text {
                text: control.__locale.dayName(styleData.dayOfWeek,control.dayOfWeekFormat)
                color: "white"
                anchors.centerIn: parent
                font.pixelSize: 12.0 * uiScale
            }
        }
    }
}
