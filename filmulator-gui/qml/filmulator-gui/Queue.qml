import QtQuick 2.12
import QtQuick.Layouts 1.12
import QtQml.Models 2.12
import "gui_components"
import "getRoot.js" as GetRootObject
import "colors.js" as Colors

Item {
    id: root
    property real uiScale: 1

    //The url gives us the resource name of the most recently produced image.
    property string url: ""

    property real thirdWidth: width/3

    property real queueCount: queueModel.queueSize

    property bool dragging: false
    property bool wasDragScrolling: false

    property real oldX
    property real newX

    property bool onEditTab

    property bool helpMode: false

    property bool rightClicked: false

    signal doubleClicked()

    onDraggingChanged: {
        //Always reset it back to off when the state changes.
        wasDragScrolling = false
    }

    onNewXChanged: {
        //Only do this if we were already dragScrolling.
        //This is to prevent the initial drag activation from initiating scrolling.

        //We want it to ramp up slowly, so that even towards the edges, you can swap two adjacent images without trouble.
        // Hence, the (-velocity/10 +-430)/uiScale term.

        //We cap the speed at the flick strength, which gets stronger from 1/3 from the edge towards the edge.

        //Additionally, the bigger the window, the faster we can go without the empty gap flying off the edge, so
        // flickStrength is multiplied by the square root of the window size relative to the 720p window size.
        var velocity = listView.horizontalVelocity
        var flickStrength
        if (newX >= oldX && newX >= 2*thirdWidth && wasDragScrolling) {
            flickStrength = Math.sqrt(width/(1280*uiScale))*(newX-2*thirdWidth)/thirdWidth
            listView.flick(uiScale*Math.max((-velocity/10 -430)/uiScale, -3000*flickStrength),0)
        }
        else if (newX <= oldX && newX <= thirdWidth && wasDragScrolling) {
            flickStrength = Math.sqrt(width/(1280*uiScale))*(thirdWidth-newX)/thirdWidth
            listView.flick(uiScale*Math.min((-velocity/10 +430)/uiScale, 3000*flickStrength),0)
        }
        else {
            listView.flick(0,0)
        }
        oldX = newX
        wasDragScrolling = true
    }

    signal tooltipWanted(string text, int x, int y)

    GridView { //There is a bug in ListView that makes scrolling not smooth.
        id: listView
        x: 0
        y: 0
        width: parent.width
        height: parent.height - 5*uiScale
        flow: GridView.FlowTopToBottom
        layoutDirection: Qt.LeftToRight
        cacheBuffer: 10
        cellWidth: root.height
        cellHeight: root.height

        property real trueContentWidth: Math.max(width, cellWidth*queueModel.queueSize)

        boundsBehavior: Flickable.StopAtBounds
        flickDeceleration: 6000 * uiScale
        maximumFlickVelocity: 10000 * Math.sqrt(uiScale)

        onMovingChanged: { //reset params after mouse scrolling
            if (!moving) {
                flickDeceleration = 6000 * uiScale
                maximumFlickVelocity = 10000 * Math.sqrt(uiScale)
            }
        }

        displaced: Transition {
            NumberAnimation {
                properties: "x"
                easing.type: Easing.OutQuad
                duration: 200
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

            delegate: MouseArea {
                id: delegateRoot
                width: listView.height
                height: listView.height

                property int visualIndex: DelegateModel.itemsIndex
                property int oldVisualIndex

                /*DEBUG for QUEUE DRAG&DROP
                Text {
                    color: "white"
                    text: visualIndex
                }
                Text {
                    color: "white"
                    y: 10
                    text: QTsearchID
                }*/

                z: (held || queueDelegate.rightClicked) ? 1 : 0

                property bool held: false

                property alias widthScale: queueDelegate.widthScale

                //Tell it to move the queueDelegate
                drag.target: held ? queueDelegate : undefined
                drag.axis: Drag.XAxis

                acceptedButtons: Qt.LeftButton
                onPressAndHold: {
                    held = true
                    oldVisualIndex = visualIndex
                    root.dragging = true
                }
                onReleased: {
                    held = false
                    queueModel.move(queueDelegate.searchID, delegateRoot.visualIndex)
                    root.dragging = false
                }

                onDoubleClicked: {
                    //Swap any precomputed pipelines
                    //First we want to find which is the old and which is the new index
                    var oldPosition = queueModel.getActivePosition(paramManager.imageIndex)
                    var newPosition = queueModel.getActivePosition(QTsearchID)
                    var nextID = ""
                    if (newPosition === oldPosition || queueModel.getQueueSize() === 1) {
                        //no change shuffle
                    } else if (newPosition < oldPosition) {
                        if (newPosition !== 0.0) {
                            nextID = queueModel.getPrev(QTsearchID)
                        } else {//we know it's not the last item on the queue
                            nextID = queueModel.getNext(QTsearchID)
                        }
                    } else if (newPosition > oldPosition) {
                        if (newPosition !== 1.0) {
                            nextID = queueModel.getNext(QTsearchID)
                        } else {//we know it's not also the first item on the queue
                            nextID = queueModel.getPrev(QTsearchID)
                        }
                    }
                    //do the shuffle
                    filmProvider.prepareShuffle(QTsearchID, nextID);

                    console.log("New image: " + QTsearchID)
                    paramManager.selectImage(QTsearchID)
                    root.doubleClicked()
                }

                QueueDelegate {
                    id: queueDelegate
                    anchors.horizontalCenter: parent.horizontalCenter
                    anchors.verticalCenter: delegateRoot.verticalCenter
                    dim: listView.height
                    rootDir: organizeModel.thumbDir()

                    selectedID: paramManager.imageIndex
                    searchID: QTsearchID
                    processed: QTprocessed
                    exported: QTexported
                    markedForOutput: QToutput
                    rating: STrating

                    queueIndex: QTindex

                    //This is the location of the latest image from the film image provider.
                    freshURL: root.url

                    onRefresh: {
                        console.log('refreshing queue')
                        queueModel.updateAll()
                    }


                    MouseArea {
                        anchors.fill: parent
                        acceptedButtons: Qt.RightButton
                        onClicked: {
                            loadMenu.sourceComponent = rightClickMenu
                            queueDelegate.rightClicked = true
                            root.rightClicked = true
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
                                    root.rightClicked = false
                                    loadMenu.sourceComponent = undefined
                                }
                                onWheel: {
                                    queueDelegate.rightClicked = false
                                    root.rightClicked = false
                                    loadMenu.sourceComponent = undefined
                                    queueModel.updateAll();
                                }
                            }
                            ColumnLayout {
                                id: menuLayout
                                spacing: 0 * root.uiScale
                                x: Math.min(Math.max(0,-queueDelegate.mapToItem(null,0,0).x), sizer.mapToItem(queueDelegate,sizer.width,0).x - width)
                                anchors.bottom: parent.top
                                z: 2
                                width: 200 * root.uiScale

                                ToolButton {
                                    id: forgetButton
                                    property bool active: false
                                    text: active ? qsTr("Are you sure?") : qsTr("...Wait a moment...")
                                    width: parent.width
                                    z: 2
                                    uiScale: root.uiScale

                                    onTriggered: {
                                        if (forgetButton.active) {
                                            queueModel.batchForget()
                                            forgetButton.active = false
                                            queueDelegate.rightClicked = false
                                            root.rightClicked = false
                                            loadMenu.sourceComponent = undefined
                                        }
                                        else {
                                            forgetCover.visible = true
                                            forgetButton.active = false
                                            forgetDelay.stop()
                                        }
                                    }

                                    Timer {
                                        id: forgetDelay
                                        interval: 1000
                                        onTriggered: {
                                            forgetButton.active = true
                                            forgetTimeout.start()
                                        }
                                    }
                                    Timer {
                                        id: forgetTimeout
                                        interval: 5000
                                        onTriggered: {
                                            forgetCover.visible = true
                                            forgetButton.active = false
                                        }
                                    }

                                    ToolButton {
                                        id: forgetCover
                                        text: qsTr("Forget marked photos")
                                        tooltipText: qsTr("Remove marked photos that are in the queue from the database. The files will not be deleted.")
                                        anchors.fill: parent
                                        uiScale: root.uiScale
                                        onTriggered: {
                                            forgetButton.active = false
                                            forgetCover.visible = false
                                            forgetDelay.start()
                                        }
                                        tooltipInstant: root.helpMode
                                        Component.onCompleted: {
                                            forgetCover.tooltipWanted.connect(root.tooltipWanted)
                                        }
                                    }
                                }
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
                                            clearQueue.active = false
                                            queueDelegate.rightClicked = false
                                            root.rightClicked = false
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
                                        root.rightClicked = false
                                        loadMenu.sourceComponent = undefined
                                    }
                                    uiScale: root.uiScale
                                }
                                ToolButton {
                                    id: copyAll
                                    text: qsTr("Copy all settings")
                                    tooltipText: qsTr("Copy the processing parameters from this image, until you switch images or change processing parameters.")
                                    width: parent.width
                                    z: 2
                                    onTriggered: {
                                        paramManager.copyAll(QTsearchID)
                                        queueDelegate.rightClicked = false
                                        root.rightClicked = false
                                        loadMenu.sourceComponent = undefined
                                    }
                                    tooltipInstant: root.helpMode
                                    Component.onCompleted: {
                                        copyAll.tooltipWanted.connect(root.tooltipWanted)
                                    }
                                    uiScale: root.uiScale
                                }
                                ToolButton {
                                    id: paste
                                    text: qsTr("Paste settings")
                                    tooltipText: qsTr("If you copy and then do anything other than paste, pasting will not be available.")
                                    width: parent.width
                                    z: 2
                                    notDisabled: paramManager.pasteable
                                    onTriggered: {
                                        paramManager.paste(QTsearchID)
                                        //notify the film image provider that this image was pasted to
                                        //so that it can tell the paramManager to reselect the image
                                        filmProvider.refreshParams(QTsearchID)
                                        queueDelegate.rightClicked = false
                                        root.rightClicked = false
                                        loadMenu.sourceComponent = undefined
                                    }
                                    tooltipInstant: root.helpMode
                                    Component.onCompleted: {
                                        paste.tooltipWanted.connect(root.tooltipWanted)
                                    }
                                    uiScale: root.uiScale
                                }
                                Item {
                                    //row layouts have rounding issues at high item counts if you specify size
                                    //if you use Layouts.fillWidth then the gaps are too big
                                    id: rateRow
                                    width: parent.width
                                    height: 30 * root.uiScale
                                    z: 2

                                    property int buttonCount: 7
                                    property real buttonWidth: parent.width/buttonCount

                                    ToolButton {
                                        id: rateNegative
                                        width: parent.buttonWidth
                                        x: 0 * parent.buttonWidth
                                        text: qsTr("X")
                                        tooltipText: qsTr("Mark this photo for forgetting or for deletion")
                                        notDisabled: STrating >= 0 //-6 through -1 should be deletion, mapped to 5 through 0 rating, to preserve rating when swapping between deletion and non-deletion
                                        uiScale: root.uiScale
                                        onTriggered: {
                                            organizeModel.setRating(QTsearchID, -1)
                                            queueDelegate.rightClicked = false
                                            root.rightClicked = false
                                            loadMenu.sourceComponent = undefined
                                        }
                                        tooltipInstant: root.helpMode
                                        Component.onCompleted: {
                                            rateNegative.tooltipWanted.connect(root.tooltipWanted)
                                        }
                                    }
                                    ToolButton {
                                        id: rate0
                                        width: parent.buttonWidth
                                        x: 1 * parent.buttonWidth
                                        text: qsTr("0")
                                        tooltipText: qsTr("Rate this 0 stars")
                                        notDisabled: 0 != STrating
                                        uiScale: root.uiScale
                                        onTriggered: {
                                            organizeModel.setRating(QTsearchID, 0)
                                            queueDelegate.rightClicked = false
                                            root.rightClicked = false
                                            loadMenu.sourceComponent = undefined
                                        }
                                        tooltipInstant: root.helpMode
                                        Component.onCompleted: {
                                            rate0.tooltipWanted.connect(root.tooltipWanted)
                                        }
                                    }
                                    ToolButton {
                                        id: rate1
                                        width: parent.buttonWidth
                                        x: 2 * parent.buttonWidth
                                        text: qsTr("1")
                                        tooltipText: qsTr("Rate this 1 star")
                                        notDisabled: 1 != STrating
                                        uiScale: root.uiScale
                                        onTriggered: {
                                            organizeModel.setRating(QTsearchID, 1)
                                            queueDelegate.rightClicked = false
                                            root.rightClicked = false
                                            loadMenu.sourceComponent = undefined
                                        }
                                        tooltipInstant: root.helpMode
                                        Component.onCompleted: {
                                            rate1.tooltipWanted.connect(root.tooltipWanted)
                                        }
                                    }
                                    ToolButton {
                                        id: rate2
                                        width: parent.buttonWidth
                                        x: 3 * parent.buttonWidth
                                        text: qsTr("2")
                                        tooltipText: qsTr("Rate this 2 stars")
                                        notDisabled: 2 != STrating
                                        uiScale: root.uiScale
                                        onTriggered: {
                                            organizeModel.setRating(QTsearchID, 2)
                                            queueDelegate.rightClicked = false
                                            root.rightClicked = false
                                            loadMenu.sourceComponent = undefined
                                        }
                                        tooltipInstant: root.helpMode
                                        Component.onCompleted: {
                                            rate2.tooltipWanted.connect(root.tooltipWanted)
                                        }
                                    }
                                    ToolButton {
                                        id: rate3
                                        width: parent.buttonWidth
                                        x: 4 * parent.buttonWidth
                                        text: qsTr("3")
                                        tooltipText: qsTr("Rate this 3 stars")
                                        notDisabled: 3 != STrating
                                        uiScale: root.uiScale
                                        onTriggered: {
                                            organizeModel.setRating(QTsearchID, 3)
                                            queueDelegate.rightClicked = false
                                            root.rightClicked = false
                                            loadMenu.sourceComponent = undefined
                                        }
                                        tooltipInstant: root.helpMode
                                        Component.onCompleted: {
                                            rate3.tooltipWanted.connect(root.tooltipWanted)
                                        }
                                    }
                                    ToolButton {
                                        id: rate4
                                        width: parent.buttonWidth
                                        x: 5 * parent.buttonWidth
                                        text: qsTr("4")
                                        tooltipText: qsTr("Rate this 4 stars")
                                        notDisabled: 4 != STrating
                                        uiScale: root.uiScale
                                        onTriggered: {
                                            organizeModel.setRating(QTsearchID, 4)
                                            queueDelegate.rightClicked = false
                                            root.rightClicked = false
                                            loadMenu.sourceComponent = undefined
                                        }
                                        tooltipInstant: root.helpMode
                                        Component.onCompleted: {
                                            rate4.tooltipWanted.connect(root.tooltipWanted)
                                        }
                                    }
                                    ToolButton {
                                        id: rate5
                                        width: parent.buttonWidth
                                        x: 6 * parent.buttonWidth
                                        text: qsTr("5")
                                        tooltipText: qsTr("Rate this 5 stars")
                                        notDisabled: 5 != STrating
                                        uiScale: root.uiScale
                                        onTriggered: {
                                            organizeModel.setRating(QTsearchID, 5)
                                            queueDelegate.rightClicked = false
                                            root.rightClicked = false
                                            loadMenu.sourceComponent = undefined
                                        }
                                        tooltipInstant: root.helpMode
                                        Component.onCompleted: {
                                            rate5.tooltipWanted.connect(root.tooltipWanted)
                                        }
                                    }
                                    Shortcut {
                                        id: rightClickDelete
                                        sequence: "x"
                                        enabled: queueDelegate.rightClicked
                                        onActivated: {
                                            organizeModel.markDeletion(QTsearchID)
                                            queueDelegate.rightClicked = false
                                            root.rightClicked = false
                                            loadMenu.sourceComponent = undefined
                                        }
                                    }
                                    Shortcut {
                                        id: rightClickZero
                                        sequence: "0"
                                        enabled: queueDelegate.rightClicked
                                        onActivated: {
                                            organizeModel.setRating(QTsearchID, 0)
                                            queueDelegate.rightClicked = false
                                            root.rightClicked = false
                                            loadMenu.sourceComponent = undefined
                                        }
                                    }
                                    Shortcut {
                                        id: rightClickOne
                                        sequence: "1"
                                        enabled: queueDelegate.rightClicked
                                        onActivated: {
                                            organizeModel.setRating(QTsearchID, 1)
                                            queueDelegate.rightClicked = false
                                            root.rightClicked = false
                                            loadMenu.sourceComponent = undefined
                                        }
                                    }
                                    Shortcut {
                                        id: rightClickTwo
                                        sequence: "2"
                                        enabled: queueDelegate.rightClicked
                                        onActivated: {
                                            organizeModel.setRating(QTsearchID, 2)
                                            queueDelegate.rightClicked = false
                                            root.rightClicked = false
                                            loadMenu.sourceComponent = undefined
                                        }
                                    }
                                    Shortcut {
                                        id: rightClickThree
                                        sequence: "3"
                                        enabled: queueDelegate.rightClicked
                                        onActivated: {
                                            organizeModel.setRating(QTsearchID, 3)
                                            queueDelegate.rightClicked = false
                                            root.rightClicked = false
                                            loadMenu.sourceComponent = undefined
                                        }
                                    }
                                    Shortcut {
                                        id: rightClickFour
                                        sequence: "4"
                                        enabled: queueDelegate.rightClicked
                                        onActivated: {
                                            organizeModel.setRating(QTsearchID, 4)
                                            queueDelegate.rightClicked = false
                                            root.rightClicked = false
                                            loadMenu.sourceComponent = undefined
                                        }
                                    }
                                    Shortcut {
                                        id: rightClickFive
                                        sequence: "5"
                                        enabled: queueDelegate.rightClicked
                                        onActivated: {
                                            organizeModel.setRating(QTsearchID, 5)
                                            queueDelegate.rightClicked = false
                                            root.rightClicked = false
                                            loadMenu.sourceComponent = undefined
                                        }
                                    }
                                    Shortcut {
                                        id: rightClickUp
                                        sequence: StandardKey.MoveToPreviousLine
                                        enabled: queueDelegate.rightClicked
                                        onActivated: {
                                            organizeModel.incrementRating(QTsearchID, 1)
                                            queueDelegate.rightClicked = false
                                            root.rightClicked = false
                                            loadMenu.sourceComponent = undefined
                                        }
                                    }
                                    Shortcut {
                                        id: rightClickDown
                                        sequence: StandardKey.MoveToNextLine
                                        enabled: queueDelegate.rightClicked
                                        onActivated: {
                                            organizeModel.incrementRating(QTsearchID, -1)
                                            queueDelegate.rightClicked = false
                                            root.rightClicked = false
                                            loadMenu.sourceComponent = undefined
                                        }
                                    }
                                }
                                Item {
                                    id: rightClickBottomSpacer
                                    implicitHeight: 2*uiScale
                                }
                            }
                        }
                    }

                    held: delegateRoot.held
                    Drag.active: held
                    Drag.source: delegateRoot
                    Drag.hotSpot.x: width/2
                    Drag.hotSpot.y: height/2
                    onHeldChanged: {
                        if (held === false) {
                            Drag.drop()
                        }
                    }

                    states: [
                        State {
                            when: delegateRoot.held
                            ParentChange {
                                target: queueDelegate
                                parent: listView
                            }
                            AnchorChanges {
                                target: queueDelegate
                                anchors.horizontalCenter: undefined
                            }
                        }
                    ]
                }

                DropArea {
                    id: dropArea
                    anchors.fill: parent

                    onEntered: {
                        var source = drag.source.visualIndex
                        var dest = delegateRoot.visualIndex
                        //Tell the root that the cursor has moved to initiate scrolling.
                        root.newX = mapToItem(root,drag.x,drag.y).x
                        visualModel.items.move(source, dest)
                    }

                    onPositionChanged: {
                        //Tell the root that the cursor has moved to initiate scrolling.
                        root.newX = mapToItem(root,drag.x,drag.y).x
                    }
                }
            }
        }

        //Once we get incremental updates, this should also probably go away.
        Connections {
            target: queueModel
            function onQueueChanged() {
                var xPos = listView.contentX
                queueModel.setQueueQuery()
                listView.contentX = xPos
                listView.returnToBounds()
            }
        }

        //This one will have to go away when we get proper updates.
        Connections {
            target: importModel
            function onImportChanged() {
                var xPos = listView.contentX
                queueModel.setQueueQuery()
                listView.contentX = xPos
                listView.returnToBounds()
            }
        }
    }

    Rectangle {
        id: noQueueBox
        width: 700 * uiScale
        height: Math.min(noQueueText.contentHeight + 25 * uiScale, parent.height)
        anchors.centerIn: parent
        color: Colors.darkGray
        border.color: Colors.lowGray
        border.width: 2 * uiScale
        radius: 10 * uiScale
        visible: root.queueCount < 1

        Text {
            id: noQueueText
            width: 675 * uiScale
            anchors.centerIn: parent
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            color: "white"
            font.pixelSize: 14.0 * uiScale
            wrapMode: Text.Wrap
            text: qsTr("Work Queue is empty. Import new photos with the Import tab or load existing photos by double-clicking on them in the Organize tab.")
        }
    }

    Item {
        id: scrollbarHolder
        x: 0
        y: parent.height-15*uiScale
        width: parent.width
        height: 15*uiScale

        Rectangle {
            id: scrollbarBackground
            color: Colors.darkGray
            opacity: 0

            y: parent.height-height - 1 * uiScale
            height: 3 * uiScale

            x: 0
            width: parent.width

            transitions: Transition {
                NumberAnimation {
                    property: "height"
                    duration: 200
                }
                NumberAnimation {
                    property: "opacity"
                    duration: 200
                }
            }
            states: State {
                name: "hovered"
                when: scrollbarMouseArea.containsMouse || scrollbarMouseArea.pressed
                PropertyChanges {
                    target: scrollbarBackground
                    height: 12 * uiScale
                    opacity: 0.5
                }
            }
        }

        Rectangle {
            id: scrollbar
            color: scrollbarMouseArea.pressed ? Colors.medOrange : scrollbarMouseArea.containsMouse ? Colors.weakOrange : Colors.middleGray
            radius: 1.5*uiScale

            y: parent.height-height - 1 * uiScale
            height: 3 * uiScale

            x: 1 * uiScale + (0.99*listView.contentX/listView.trueContentWidth) * (parent.width - 2*uiScale)
            width: (0.99*listView.width/listView.trueContentWidth + 0.01) * (parent.width - 2*uiScale)

            transitions: Transition {
                NumberAnimation {
                    property: "height"
                    duration: 200
                }
            }
            states: State {
                name: "hovered"
                when: scrollbarMouseArea.containsMouse || scrollbarMouseArea.pressed
                PropertyChanges {
                    target: scrollbar
                    height: 12 * uiScale
                }
            }
        }
        MouseArea {
            id: scrollbarMouseArea
            anchors.fill: parent
            hoverEnabled: true
            acceptedButtons: Qt.LeftButton
            onWheel: {
                //We have to duplicate the wheelstealer one because this has higher priority for some reason.
                //Set the scroll deceleration and max speed higher for wheel scrolling.
                //It should be reset when the view stops moving.
                //For now, this is 10x higher than standard.
                var deceleration = 6000 * 10
                listView.flickDeceleration = deceleration * uiScale
                listView.maximumFlickVelocity = 10000 * Math.sqrt(uiScale*10)

                var velocity = listView.horizontalVelocity/uiScale
                var newVelocity = velocity

                var distance = 100
                if (wheel.angleDelta.y > 0 && !listView.atXBeginning && !root.dragging) {
                    //Leftward; up on the scroll wheel.

                    //This formula makes each click of the wheel advance the 'target' a fixed distance.
                    //We use the angle delta to handle multi-size scrolling like smooth scrolling touchpads.
                    //We scale by uiScale.

                    //If we're stopped or already moving leftward, add to the flick velocity. If we're moving rightward, then stop.

                    //How much to add to the flick velocity?
                    //First, we want to know where it'll stop currently, which is
                    //d = at^2 + bt + c
                    //when
                    //  a, the acceleration, is -deceleration (-D)
                    //  b, the initial velocity, is velocity (V)
                    //  c is 0
                    //and t is the value where velocity is 0
                    //0 = 2at + b
                    //t = -b/2a
                    //and thus, plugging that t into the first equation,
                    //d = b^2/(4a) - b^2/(2a) = - b^2/(4a)
                    //d1 = V1^2 / (4D)
                    //Now we want to increase that by an increment.
                    //d2 = d1 + delta = V2^2 / (4D)
                    //V1^2/(4D) + delta = V2^2/(4D)
                    //V2 = sqrt((V1^2/(4D) + delta) * 4D)

                    newVelocity = uiScale*(velocity <= 0 ? Math.sqrt((velocity*velocity/(4*deceleration) + distance*wheel.angleDelta.y/(120))*4*deceleration) : 0)

                    //limit to the max flick velocity
                    newVelocity = Math.min(newVelocity, listView.maximumFlickVelocity)

                    //the flick logic sometimes sends the view flying like crazy if it's already moving quickly
                    //so we set it to something low (nonzero so that deceleration won't get reset)
                    listView.flick(1,0)

                    listView.flick(newVelocity, 0)
                } else if (wheel.angleDelta.y < 0 && !listView.atXEnd && !root.dragging) {
                    //Rightward; down on the scroll wheel.
                    newVelocity = uiScale*(velocity >= 0 ? Math.sqrt((velocity*velocity/(4*deceleration) + distance*wheel.angleDelta.y/(-120))*4*deceleration) : 0)
                    newVelocity = -Math.min(newVelocity, listView.maximumFlickVelocity)
                    listView.flick(-1,0)
                    listView.flick(newVelocity, 0)
                }
            }

            property bool overDragThresh: false
            property real pressX
            property real initialX
            onPositionChanged: {
                if (pressed) {
                    var deltaX = mouse.x - pressX
                    var scrollWidth = scrollbarMouseArea.width - scrollbar.width - 2*uiScale
                    var relativeDelta = deltaX / scrollWidth
                    var scrollMargin = listView.trueContentWidth - listView.width
                    var temp = initialX + relativeDelta*scrollMargin
                    listView.contentX = Math.max(0, Math.min(scrollMargin, initialX + relativeDelta * scrollMargin))
                }
            }

            onPressed: {
                preventStealing = true
                listView.cancelFlick()
                pressX = mouse.x
                initialX = listView.contentX
            }
            onReleased: {
                preventStealing = false
            }
        }
    }

    //Custom scrolling implementation.
    //It's disabled while you drag an image.
    MouseArea {
        id: wheelstealer
        anchors.fill: listView
        acceptedButtons: Qt.NoButton
        onWheel: {
            //Set the scroll deceleration and max speed higher for wheel scrolling.
            //It should be reset when the view stops moving.
            //For now, this is 10x higher than standard.
            var deceleration = 6000 * 10
            listView.flickDeceleration = deceleration * uiScale
            listView.maximumFlickVelocity = 10000 * Math.sqrt(uiScale*10)

            var velocity = listView.horizontalVelocity/uiScale
            var newVelocity = velocity

            var distance = 100
            if (wheel.angleDelta.y > 0 && !listView.atXBeginning && !root.dragging) {
                //Leftward; up on the scroll wheel.
                newVelocity = uiScale*(velocity <= 0 ? Math.sqrt((velocity*velocity/(4*deceleration) + distance*wheel.angleDelta.y/(120))*4*deceleration) : 0)
                newVelocity = Math.min(newVelocity, listView.maximumFlickVelocity)
                listView.flick(1,0)
                listView.flick(newVelocity, 0)
            } else if (wheel.angleDelta.y < 0 && !listView.atXEnd && !root.dragging) {
                //Rightward; down on the scroll wheel.
                newVelocity = uiScale*(velocity >= 0 ? Math.sqrt((velocity*velocity/(4*deceleration) + distance*wheel.angleDelta.y/(-120))*4*deceleration) : 0)
                newVelocity = -Math.min(newVelocity, listView.maximumFlickVelocity)
                listView.flick(-1,0)
                listView.flick(newVelocity, 0)
            }
        }
    }

    Shortcut {
        id: rateDelete
        sequence: "x"
        enabled: onEditTab && !root.rightClicked
        onActivated: {
            organizeModel.markDeletion(paramManager.imageIndex)
        }
    }
    Shortcut {
        id: rateZero
        sequence: "0"
        enabled: onEditTab && !root.rightClicked
        onActivated: {
            organizeModel.setRating(paramManager.imageIndex, 0)
        }
    }
    Shortcut {
        id: rateOne
        sequence: "1"
        enabled: onEditTab && !root.rightClicked
        onActivated: {
            organizeModel.setRating(paramManager.imageIndex, 1)
        }
    }
    Shortcut {
        id: rateTwo
        sequence: "2"
        enabled: onEditTab && !root.rightClicked
        onActivated: {
            organizeModel.setRating(paramManager.imageIndex, 2)
        }
    }
    Shortcut {
        id: rateThree
        sequence: "3"
        enabled: onEditTab && !root.rightClicked
        onActivated: {
            organizeModel.setRating(paramManager.imageIndex, 3)
        }
    }
    Shortcut {
        id: rateFour
        sequence: "4"
        enabled: onEditTab && !root.rightClicked
        onActivated: {
            organizeModel.setRating(paramManager.imageIndex, 4)
        }
    }
    Shortcut {
        id: rateFive
        sequence: "5"
        enabled: onEditTab && !root.rightClicked
        onActivated: {
            organizeModel.setRating(paramManager.imageIndex, 5)
        }
    }
    Shortcut {
        id: rateUp
        sequence: StandardKey.MoveToPreviousLine
        enabled: onEditTab && !root.rightClicked
        onActivated: {
            organizeModel.incrementRating(paramManager.imageIndex, 1)
        }
    }
    Shortcut {
        id: rateDown
        sequence: StandardKey.MoveToNextLine
        enabled: onEditTab && !root.rightClicked
        onActivated: {
            organizeModel.incrementRating(paramManager.imageIndex, -1)
        }
    }
    Shortcut {
        id: prevImage
        sequence: StandardKey.MoveToPreviousChar
        enabled: onEditTab
        onActivated: {
            if (!root.dragging) {
                //Swap any precomputed pipelines
                var newID = queueModel.getPrev(paramManager.imageIndex)
                var nextID = queueModel.getPrev(newID)
                if (newID !== "") {
                    filmProvider.prepareShuffle(newID, nextID);

                    if (newID !== paramManager.imageIndex) {
                        paramManager.selectImage(newID)
                        var selectedPosition = queueModel.getActivePosition(newID)
                        var scrollMargin = listView.trueContentWidth - listView.width
                        listView.contentX = Math.max(0, Math.min(scrollMargin, selectedPosition * scrollMargin))
                    }
                }
            }
        }
    }
    Shortcut {
        id: nextImage
        sequence: StandardKey.MoveToNextChar
        enabled: onEditTab
        onActivated: {
            if (!root.dragging) {
                //Swap any precomputed pipelines
                var newID = queueModel.getNext(paramManager.imageIndex)
                var nextID = queueModel.getNext(newID)
                if (newID !== "") {
                    filmProvider.prepareShuffle(newID, nextID);

                    if (newID !== paramManager.imageIndex) {
                        paramManager.selectImage(newID)
                        var selectedPosition = queueModel.getActivePosition(newID)
                        var scrollMargin = listView.trueContentWidth - listView.width
                        listView.contentX = Math.max(0, Math.min(scrollMargin, selectedPosition * scrollMargin))
                    }
                }
            }
        }
    }
}
