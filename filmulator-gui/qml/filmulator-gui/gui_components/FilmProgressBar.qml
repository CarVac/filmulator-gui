import QtQuick 2.2
import QtQuick.Controls 1.2
import QtQuick.Controls.Styles 1.2

Item {
    id: root
    width: 200
    height: 30
    property alias value: progressBar.value

    property real __padding: 2

    ProgressBar {
        id: progressBar
        orientation: Qt.Horizontal
        indeterminate: false
        visible: true
        x: __padding
        y: __padding

        style: ProgressBarStyle {
            background: Rectangle {
                implicitWidth: root.width - __padding * 2
                implicitHeight: root.height - __padding * 2
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
