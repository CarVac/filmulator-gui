import QtQuick 2.2
import QtQuick.Controls 1.2
import QtQuick.Controls.Styles 1.2
import QtQuick.Layouts 1.1
import "gui_components"

Item {
    id: root

    ListView {
        id: listView
        anchors.fill: parent
        orientation: Qt.Horizontal
        layoutDirection: Qt.LeftToRight

        delegate: QueueDelegate {
            dim: root.height
            rootDir: organizeModel.thumbDir()

            searchID: QTsearchID

            MouseArea {
                anchors.fill: parent
                onClicked: parent.ListView.view.currentIndex = index
                onDoubleClicked: {
                    console.log("New image: " + QTsearchID)
                    paramManager.selectImage(QTsearchID)
                }
            }

            MouseArea {
                anchors.fill: parent
                acceptedButtons: Qt.RightButton
                onClicked: {
                    rightClickMenu.popup()
                }
            }

            Menu {
                id: rightClickMenu
                visible: false
                MenuItem {
                    text: qsTr("Remove from queue")
                    onTriggered: {
                        queueModel.deQueue(QTsearchID)
                    }
                }

                style: MenuStyle {
                    frame: Rectangle {
                        color: "#303030"
                        border.color: "#808080"
                        border.width: 2
                    }
                    itemDelegate.label: Text {
                        color: styleData.enabled ? "#FFFFFF" : "#808080"
                        text: styleData.text
                    }
                    itemDelegate.background: Rectangle {
                        color: styleData.enabled ? (styleData.selected ? "#000000" : "#303030") : "#303030"
                    }
                }
            }
        }

        Component.onCompleted: {
            queueModel.setQueueQuery()
            listView.model = queueModel
        }
        Connections {
            target: queueModel
            onQueueChanged: {
                var xPos = listView.contentX
                queueModel.setQueueQuery()
                listView.contentX = xPos
            }
        }
    }
}
