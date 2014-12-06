import QtQuick 2.2
import QtQuick.Controls 1.2
import QtQuick.Controls.Styles 1.2
import QtQuick.Layouts 1.1
import "gui_components"
import "colors.js" as Colors

Item {
    id: root
    property real uiScale: 1

//    ListView {
    GridView { //There is a bug in ListView that makes scrolling not smooth.
        id: listView
        anchors.fill: parent
        flow: GridView.FlowTopToBottom
        //orientation: Qt.Horizontal
        layoutDirection: Qt.LeftToRight
        cacheBuffer: 10
        cellWidth: root.height * 0.9375
        cellHeight: root.height

        boundsBehavior: Flickable.StopAtBounds
        flickDeceleration: 3000
        maximumFlickVelocity: 10000 * uiScale

        model: queueModel

        delegate: QueueDelegate {
            dim: root.height
            rootDir: organizeModel.thumbDir()

            selectedID: paramManager.imageIndex
            searchID: QTsearchID
            processed: QTprocessed
            exported: QTexported

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
                        color: Colors.darkGray
                        border.color: Colors.middleGray
                        border.width: 2 * uiScale
                    }
                    itemDelegate.label: Text {
                        color: styleData.enabled ? "white" : Colors.middleGray
                        text: styleData.text

                    }
                    itemDelegate.background: Rectangle {
                        color: styleData.enabled ? (styleData.selected ? "black" : Colors.darkGray) : Colors.darkGray
                    }
                }
            }
        }

        Connections {
            target: queueModel
            onQueueChanged: {
                var xPos = listView.contentX
                queueModel.setQueueQuery()
                listView.contentX = xPos
            }
            //onDataChanged: console.log("queue.qml data changed")
        }
    }

    MouseArea {
        id: wheelstealer
        //Custom scrolling implementation.
        anchors.fill: listView
        acceptedButtons: Qt.NoButton
        onWheel: {
            var velocity = listView.horizontalVelocity
            if (wheel.angleDelta.y > 0 && !listView.atXBeginning) {
                //Leftward; up on the scroll wheel.
                //This formula makes each click of the wheel advance the 'target' a fixed distance.
                listView.flick(velocity < 0 ? Math.sqrt(velocity*velocity + 2000000) : 1000, 0)
            }
            if (wheel.angleDelta.y < 0 && !listView.atXEnd) {
                //Rightward; down on the scroll wheel.
                listView.flick(velocity > 0 ? -Math.sqrt(velocity*velocity + 2000000) : -1000, 0)
            }
        }
    }
}
