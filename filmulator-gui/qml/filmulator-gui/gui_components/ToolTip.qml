import QtQuick 2.3

Rectangle {
    id: root
    anchors.fill: parent
    color: "#00000000"
    property string tooltipText
    property real milliSecondDelay: 1000

    signal tooltipWanted(string text, int x, int y)

    Timer {
        id: tooltipTimer
        interval: parent.milliSecondDelay
        onTriggered: {
            //Maps the mouse location to the root QML view coordinates
            var point = tooltipArea.mapToItem(root.getDocRoot(), tooltipArea.mouseX, tooltipArea.mouseY)
            root.tooltipWanted(tooltipText, point.x, point.y)
        }
    }
    function getDocRoot(){
        var docRoot = null
        if (!docRoot) {
            docRoot = root.parent
            while (docRoot.parent) {
                docRoot = docRoot.parent;
            }
        }
        return docRoot
    }

    MouseArea {
        id: tooltipArea
        anchors.fill: parent
        acceptedButtons: Qt.NoButton
        hoverEnabled: true


        onEntered: tooltipTimer.start()
        onPositionChanged: tooltipTimer.restart()
        onExited: tooltipTimer.stop()
    }
}
