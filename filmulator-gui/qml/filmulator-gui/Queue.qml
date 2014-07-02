import QtQuick 2.2
import QtQuick.Controls 1.2
import QtQuick.Layouts 1.1
import "gui_components"

Item {
    id: root

    ListView {
        id: listView
        anchors.fill: parent
        orientation: Qt.Horizontal
        layoutDirection: Qt.LeftToRight

        Text {
            id: meh
            text: "this is the queue " + organizeModel.thumbDir()
            color: "black"
        }

        delegate: QueueDelegate {
            dim: root.height
            rootDir: organizeModel.thumbDir()

            searchID: QTsearchID

            filePath: FTfilePath

            MouseArea {
                anchors.fill: parent
                onClicked: parent.ListView.view.currentIndex = index
            }
        }

        Component.onCompleted: {
            queueModel.setQueueQuery()
            console.log("Queue completed");
            console.log("height: " + listView.height)
            listView.model = queueModel
        }
    }
}
