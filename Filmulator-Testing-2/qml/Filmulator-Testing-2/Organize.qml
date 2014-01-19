import QtQuick 2.2
import QtQuick.Controls 1.1
import QtQuick.Layouts 1.1

SplitView {
    anchors.fill: parent
    orientation: Qt.Horizontal

    Rectangle {
        id: folderlist
        color: "lightblue"
        width: 100
        Layout.maximumWidth: 500
    }

    Rectangle {
        id: gridview
        color: "orange"
        Layout.fillWidth: true
    }
}
