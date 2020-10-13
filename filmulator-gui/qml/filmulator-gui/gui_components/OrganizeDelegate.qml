import QtQuick 2.12
import "../colors.js" as Colors

Rectangle {
    id: root
    property real uiScale: 1
    width: 300 * uiScale
    height: 300 * uiScale
    color: "#00000000"
    property bool isCurrentItem
    property string rootDir
    property string searchID
    property int captureTime
    property int importTime
    property int lastProcessedTime
    property int rating
    property string filename
    property int thumbWritten

    property string __thumbPath: (Qt.platform.os == "windows" ? 'file:///' : 'file://') + rootDir + '/'+ searchID.slice(0,4) + '/' + searchID + '.jpg'

    signal tooltipWanted(string text, int x, int y)
    signal selectImage()
    signal enqueueImage()
    signal rate(int ratingIn)

    property bool flashing: false

    Rectangle {
        id: backgroundRect
        anchors.fill: parent
        //When they are equal in the DB, this sometimes turned on randomly.
        //Subtracting 1 from the lastProcessedTime makes it not.
        color: flashing ? Colors.medOrange : (importTime < (lastProcessedTime-1)) ? "green" : "#000000"
        opacity: isCurrentItem ? .3 : .1
    }

    Loader {
        id: loadThumb
        asynchronous: true
        anchors.centerIn: parent
        //sourceComponent: thumbImage
    }

    Component {
        id: thumbImage
        Image {
            id: thumb
            width: 290 * uiScale
            height: 290 * uiScale
            fillMode: Image.PreserveAspectFit
            source: root.__thumbPath
            sourceSize.width: 290 * uiScale
            sourceSize.height: 290 * uiScale
            cache: false
        }
    }
    Component {
        id: thumbPlaceholder
        Text {
            id: thumbMissingText
            width: 290 * uiScale
            height: 290 * uiScale
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            text: qsTr("Image file is\nunreadable")
            color: "white"
            font.pixelSize: 12.0 * uiScale
        }
    }
    Component.onCompleted: {
        if (thumbWritten === -1) {
            loadThumb.sourceComponent = thumbPlaceholder
        }
        else {
            loadThumb.sourceComponent = thumbImage
        }
    }

    ToolTip {
        id: tooltip
        anchors.fill: loadThumb
        tooltipText: root.filename + '\n' + organizeModel.getDateTimeString(root.captureTime)
        Component.onCompleted: {
            tooltip.tooltipWanted.connect(root.tooltipWanted)
        }
    }
    Rectangle {
        id: ratingRect
        x: 0
        y: rating >= 0 ? 0 : 295*uiScale
        height: 5 * uiScale
        width: (rating >= 0) ? root.width * rating/5 : root.width
        color: ratingArea.pressed ? "#00000000" : ((rating >= 0) ? Colors.medOrange : "red")
    }
    MouseArea {
        id: imageEnqueuer
        anchors.fill: parent
        onClicked: root.selectImage()
        onDoubleClicked: {
            root.enqueueImage()
            root.flashing = true
            enqueueBlinker.start()
        }
    }
    Timer {
        id: enqueueBlinker
        interval: 200
        onTriggered: root.flashing = false
    }

    MouseArea {
        id: ratingArea
        enabled: root.isCurrentItem
        hoverEnabled: true
        x: 0
        y: 0
        width: 300 * uiScale
        height: 20 * uiScale
        onClicked: {
            root.rate(ratingArea.mouseX > root.width/10 ? Math.ceil(ratingArea.mouseX * 5 / root.width) : 0)
        }
        Rectangle {
            id: tempRatingRect
            x: 0
            y: 0
            width: Math.ceil(ratingArea.mouseX * 5 / root.width) * root.width / 5
            height: ratingArea.containsMouse ? 20 * uiScale : 0
            color: ratingArea.mouseX > root.width/10 ? Colors.medOrange : Colors.darkGray
            border.width: 2*uiScale
            border.color: Colors.medOrange
            opacity: 0.8
        }
    }
    onThumbWrittenChanged: {
        if (thumbWritten === 1) {
            loadThumb.source = ""
            loadThumb.sourceComponent = thumbImage
        }
        else if (thumbWritten === -1) {
            loadThumb.sourceComponent = thumbPlaceholder
        }
        //If it's 0, then we wait for it to become 1.
    }
}
