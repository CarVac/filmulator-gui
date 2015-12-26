import QtQuick 2.3
import QtQuick.Controls 1.3
import QtQuick.Controls.Styles 1.3
import QtQuick.Layouts 1.1
import QtQml.Models 2.1
import "gui_components"
import "getRoot.js" as GetRootObject
import "colors.js" as Colors

Item {
    id: root
    property real uiScale: 1
    property string url: ""

    signal tooltipWanted(string text, int x, int y)

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

        displaced: Transition {
            NumberAnimation {
                properties: "x"
                easing.type: Easing.OutQuad
                duration: 100
            }
        }
        add: Transition {
            NumberAnimation {
                property: "widthScale"
                from: 0
                to: 1
                easing.type: Easing.OutQuad
                duration: 100
            }
        }
        remove: Transition {
            NumberAnimation {
                property: "widthScale"
                from: 1
                to: 0
                easing.type: Easing.OutQuad
                duration: 100
            }
        }

        model: DelegateModel {
            id: visualModel
            model: queueModel

            delegate: QueueDelegate {
                id: queueDelegate
                dim: root.height
                rootDir: organizeModel.thumbDir()

                selectedID: paramManager.imageIndex
                searchID: QTsearchID
                processed: QTprocessed
                exported: QTexported
                markedForOutput: QToutput
                queueIndex: QTindex

                freshURL: root.url
                property int visualIndex: DelegateModel.itemsIndex

                MouseArea {
                    anchors.fill: parent
                    onClicked: {
                        listView.currentIndex = index
                    }
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
                            y: -154 * root.uiScale//#buttons*30+4
                            z: 2
                            width: 200 * root.uiScale

                            ToolButton {
                                id: clearQueue
                                property bool active: false
                                text: active ? qsTr("Are you sure?") : qsTr("...Wait a moment...")
                                width: parent.width
                                z: 2
                                uiScale: root.uiScale
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
                                    onTriggered: {
                                        clearQueue.active = false//just in case
                                        clearQueueCover.visible = false
                                        clearQueueDelay.start()
                                    }
                                }
                            }

                            ToolButton {
                                id: removeFromQueue
                                text: qsTr("Remove from queue")
                                width: parent.width
                                z: 2
                                onTriggered: {
                                    queueModel.deQueue(QTsearchID)
                                    queueDelegate.rightClicked = false
                                    loadMenu.sourceComponent = undefined
                                }
                                uiScale: root.uiScale
                            }
                            ToolButton {
                                id: copyAll
                                text: qsTr("Copy all settings")
                                width: parent.width
                                z: 2
                                onTriggered: {
                                    paramManager.copyAll(QTsearchID)
                                    queueDelegate.rightClicked = false
                                    loadMenu.sourceComponent = undefined
                                }
                                uiScale: root.uiScale
                            }
                            ToolButton {
                                id: paste
                                text: qsTr("Paste settings")
                                width: parent.width
                                z: 2
                                notDisabled: paramManager.pasteable
                                onTriggered: {
                                    paramManager.paste(QTsearchID)
                                    queueDelegate.rightClicked = false
                                    loadMenu.sourceComponent = undefined
                                }
                                uiScale: root.uiScale
                            }
                            RowLayout {
                                id: rate
                                spacing: 0 * root.uiScale
                                height: 30 * root.uiScale
                                z: 2

                                property real buttonWidth: (parent.width - root.uiScale/2)/6

                                ToolButton {
                                    id: rate0
                                    width: parent.buttonWidth
                                    text: qsTr("0")
                                    tooltipText: qsTr("Rate this 0 stars")
                                    notDisabled: 0 != STrating
                                    uiScale: root.uiScale
                                    onTriggered: {
                                        organizeModel.setRating(QTsearchID, 0)
                                        queueDelegate.rightClicked = false
                                        loadMenu.sourceComponent = undefined
                                    }
                                    Component.onCompleted: {
                                        rate0.tooltipWanted.connect(root.tooltipWanted)
                                    }
                                }
                                ToolButton {
                                    id: rate1
                                    width: parent.buttonWidth
                                    text: qsTr("1")
                                    tooltipText: qsTr("Rate this 1 star")
                                    notDisabled: 1 != STrating
                                    uiScale: root.uiScale
                                    onTriggered: {
                                        organizeModel.setRating(QTsearchID, 1)
                                        queueDelegate.rightClicked = false
                                        loadMenu.sourceComponent = undefined
                                    }
                                    Component.onCompleted: {
                                        rate1.tooltipWanted.connect(root.tooltipWanted)
                                    }
                                }
                                ToolButton {
                                    id: rate2
                                    width: parent.buttonWidth
                                    text: qsTr("2")
                                    tooltipText: qsTr("Rate this 2 stars")
                                    notDisabled: 2 != STrating
                                    uiScale: root.uiScale
                                    onTriggered: {
                                        organizeModel.setRating(QTsearchID, 2)
                                        queueDelegate.rightClicked = false
                                        loadMenu.sourceComponent = undefined
                                    }
                                    Component.onCompleted: {
                                        rate2.tooltipWanted.connect(root.tooltipWanted)
                                    }
                                }
                                ToolButton {
                                    id: rate3
                                    width: parent.buttonWidth
                                    text: qsTr("3")
                                    tooltipText: qsTr("Rate this 3 stars")
                                    notDisabled: 3 != STrating
                                    uiScale: root.uiScale
                                    onTriggered: {
                                        organizeModel.setRating(QTsearchID, 3)
                                        queueDelegate.rightClicked = false
                                        loadMenu.sourceComponent = undefined
                                    }
                                    Component.onCompleted: {
                                        rate3.tooltipWanted.connect(root.tooltipWanted)
                                    }
                                }
                                ToolButton {
                                    id: rate4
                                    width: parent.buttonWidth
                                    text: qsTr("4")
                                    tooltipText: qsTr("Rate this 4 stars")
                                    notDisabled: 4 != STrating
                                    uiScale: root.uiScale
                                    onTriggered: {
                                        organizeModel.setRating(QTsearchID, 4)
                                        queueDelegate.rightClicked = false
                                        loadMenu.sourceComponent = undefined
                                    }
                                    Component.onCompleted: {
                                        rate4.tooltipWanted.connect(root.tooltipWanted)
                                    }
                                }
                                ToolButton {
                                    id: rate5
                                    width: parent.buttonWidth
                                    text: qsTr("5")
                                    tooltipText: qsTr("Rate this 5 stars")
                                    notDisabled: 5 != STrating
                                    uiScale: root.uiScale
                                    onTriggered: {
                                        organizeModel.setRating(QTsearchID, 5)
                                        queueDelegate.rightClicked = false
                                        loadMenu.sourceComponent = undefined
                                    }
                                    Component.onCompleted: {
                                        rate5.tooltipWanted.connect(root.tooltipWanted)
                                    }
                                }
                            }
                        }
                    }
                }

                DropArea {
                    anchors.fill: parent
                    anchors.margins: 15
                    onEntered: {
                        visualModel.items.move(drag.source.visualIndex, queueDelegate.visualIndex)
                    }
                    onDropped: {
                        queueModel.move(queueDelegate.searchID, queueDelegate.visualIndex)
                    }
                }

            }
        }

        //Once we get incremental updates, this should also probably go away.
        Connections {
            target: queueModel
            onQueueChanged: {
                var xPos = listView.contentX
                queueModel.setQueueQuery()
                listView.contentX = xPos
            }
            //onDataChanged: console.log("queue.qml data changed")
        }

        //This one will have to go away when we get proper updates.
        Connections {
            target: importModel
            onImportChanged: {
                var xPos = listView.contentX
                queueModel.setQueueQuery()
                listView.contentX = xPos
            }
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
                //We use the angle delta to handle multi-size scrolling like smooth scrolling touchpads.
                listView.flick(velocity < 0 ? Math.sqrt(velocity*velocity + 2000000*wheel.angleDelta.y/120) : (velocity == 0 ? 500 : 0), 0)
            }
            if (wheel.angleDelta.y < 0 && !listView.atXEnd) {
                //Rightward; down on the scroll wheel.
                listView.flick(velocity > 0 ? -Math.sqrt(velocity*velocity + 2000000*wheel.angleDelta.y/(-120)) : (velocity == 0 ? -500 : 0), 0)
            }
        }
    }
}
