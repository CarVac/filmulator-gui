import QtQuick 2.2
import QtQuick.Controls 1.2
import QtQuick.Controls.Styles 1.2
import QtQuick.Layouts 1.1
import "gui_components"
import "getRoot.js" as GetRootObject
import "colors.js" as Colors

Item {
    id: root
    property real uiScale: 1
    property string url: ""

//    ListView {
    GridView { //There is a bug in ListView that makes scrolling not smooth.
        id: listView
        anchors.fill: parent
        flow: GridView.FlowTopToBottom
        //orientation: Qt.Horizontal
        layoutDirection: Qt.LeftToRight
        cacheBuffer: 10
        cellWidth: root.height
        cellHeight: root.height

        boundsBehavior: Flickable.StopAtBounds
        flickDeceleration: 3000
        maximumFlickVelocity: 10000 * uiScale

        model: queueModel

        delegate: QueueDelegate {
            id: queueDelegate
            dim: root.height
            rootDir: organizeModel.thumbDir()

            selectedID: paramManager.imageIndex
            searchID: QTsearchID
            processed: QTprocessed
            exported: QTexported

            freshURL: root.url

            MouseArea {
                anchors.fill: parent
                onClicked: queueDelegate.ListView.currentIndex = queueDelegate.index
                onDoubleClicked: {
                    console.log("New image: " + QTsearchID)
                    paramManager.selectImage(QTsearchID)
                }
            }

            MouseArea {
                anchors.fill: parent
                acceptedButtons: Qt.RightButton
                onClicked: {
                    loadMenu.sourceComponent = rightClickMenu
                    queueDelegate.rightClicked = true
                }
            }

            Loader {
                id: loadMenu
            }

            Component {
                id: rightClickMenu
                Item {
                    anchors.fill: parent
                    Item {
                        id: sizer
                        parent: GetRootObject.getDocRoot(root)
                        anchors.fill: parent
                    }

                    MouseArea {
                        id: clickCatcher
                        x: -queueDelegate.mapToItem(null,0,0).x - sizer.width
                        y: -queueDelegate.mapToItem(null,0,0).y - sizer.height
                        width: sizer.width * 3
                        height: sizer.height * 3
                        z: 1
                        acceptedButtons: Qt.AllButtons
                        onClicked: {
                            queueDelegate.rightClicked = false
                            loadMenu.sourceComponent = undefined
                        }
                        onWheel: {
                            queueDelegate.rightClicked = false
                            loadMenu.sourceComponent = undefined
                        }
                    }
                    ColumnLayout {
                        id: menuLayout
                        spacing: 0 * root.uiScale
                        x: Math.min(Math.max(0,-queueDelegate.mapToItem(null,0,0).x), sizer.mapToItem(queueDelegate,sizer.width,0).x - width)
                        y: -124 * root.uiScale//#buttons*30+4
                        z: 2
                        width: 200 * root.uiScale

                        ToolButton {
                            id: clearQueue
                            property bool active: false
                            text: active ? qsTr("Are you sure?") : qsTr("...Wait a moment...")
                            width: parent.width
                            z: 2
                            uiScale: root.uiScale
                            action: Action {
                                onTriggered: {
                                    if (clearQueue.active) {
                                        queueModel.clearQueue()
                                        queueDelegate.rightClicked = false
                                        loadMenu.sourceComponent = undefined
                                    }
                                    else {
                                        clearQueueCover.visible = true
                                        clearQueue.active = false
                                        clearQueueDelay.stop()
                                    }
                                }
                            }
                            onPressedChanged: {
                                if (clearQueue.pressed && !clearQueue.active) {
                                    clearQueue.active = false
                                    clearQueueCover.visible = true
                                    clearQueueDelay.stop()
                                }
                            }

                            Timer {
                                id: clearQueueDelay
                                interval: 1000
                                onTriggered: {
                                    clearQueue.active = true
                                    clearQueueTimeout.start()
                                }
                            }
                            Timer {
                                id: clearQueueTimeout
                                interval: 5000
                                onTriggered: {
                                    clearQueueCover.visible = true
                                    clearQueue.active = false
                                }
                            }

                            ToolButton {
                                id: clearQueueCover
                                text: qsTr("Clear entire queue")
                                anchors.fill: parent
                                uiScale: root.uiScale
                                action: Action {
                                    onTriggered: {
                                        clearQueue.active = false//just in case
                                        clearQueueCover.visible = false
                                        clearQueueDelay.start()
                                    }
                                }
                            }
                        }

                        ToolButton {
                            id: removeFromQueue
                            text: qsTr("Remove from queue")
                            width: parent.width
                            z: 2
                            action: Action {
                                onTriggered: {
                                    queueModel.deQueue(QTsearchID)
                                    queueDelegate.rightClicked = false
                                    loadMenu.sourceComponent = undefined
                                }
                            }
                            uiScale: root.uiScale
                        }
                        ToolButton {
                            id: copyAll
                            text: qsTr("Copy all settings")
                            width: parent.width
                            z: 2
                            action: Action {
                                onTriggered: {
                                    paramManager.copyAll(QTsearchID)
                                    queueDelegate.rightClicked = false
                                    loadMenu.sourceComponent = undefined
                                }
                            }
                            uiScale: root.uiScale
                        }
                        ToolButton {
                            id: paste
                            text: qsTr("Paste settings")
                            width: parent.width
                            z: 2
                            notDisabled: paramManager.pasteable
                            action: Action {
                                onTriggered: {
                                    paramManager.paste(QTsearchID)
                                    queueDelegate.rightClicked = false
                                    loadMenu.sourceComponent = undefined
                                }
                            }
                            uiScale: root.uiScale
                        }
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
            if (wheel.angleDelta.x + wheel.angleDelta.y > 0 && !listView.atXBeginning) {
                //Leftward; up on the scroll wheel.
                //This formula makes each click of the wheel advance the 'target' a fixed distance.
                listView.flick(velocity < 0 ? Math.sqrt(velocity*velocity + 2000000) : (velocity == 0 ? 500 : 0), 0)
            }
            if (wheel.angleDelta.x + wheel.angleDelta.y < 0 && !listView.atXEnd) {
                //Rightward; down on the scroll wheel.
                listView.flick(velocity > 0 ? -Math.sqrt(velocity*velocity + 2000000) : (velocity == 0 ? -500 : 0), 0)
            }
        }
    }
}
