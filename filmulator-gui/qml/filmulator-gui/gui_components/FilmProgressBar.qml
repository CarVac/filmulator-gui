import QtQuick 2.2
import QtQuick.Controls 1.2
import QtQuick.Controls.Styles 1.2

Item {
    id: root
    implicitWidth: 200
    implicitHeight: 30
    property alias value: progressBar.value

    ProgressBar {
        id: progressBar
        orientation: Qt.Horizontal
        indeterminate: false
        visible: true
        x: 1
        y: 1

        style: ProgressBarStyle {
            background: Rectangle {
                implicitWidth: 198
                implicitHeight: 28
                radius: 3
                color: "#B0B0B0"
                border.width: 1
                border.color: "#808080"
            }
            progress: Rectangle {
                color: "#FF9922"
                border.color: "#A87848"
                radius: 3
            }
        }
    }
}
