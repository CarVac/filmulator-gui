import QtQuick 2.3

Rectangle {
    id: root
    property real uiScale: 1
    width: 300 * uiScale
    height: 300 * uiScale
    color: "#00000000"
    property string rootDir
    property string searchID
    property int importTime
    property int lastProcessedTime

    property string __thumbPath: rootDir + '/'+ searchID.slice(0,4) + '/' + searchID + '.jpg'


    Rectangle {
        anchors.fill: parent
        color: (importTime < lastProcessedTime) ? "green" : "#00000000"
        opacity: .3
    }

    Loader {
        id: loadThumb
        asynchronous: true
        anchors.fill: parent
        //sourceComponent: thumbImage
    }

    Component {
        id: thumbImage
        Image {
            id: thumb
            anchors.fill: parent
            fillMode: Image.PreserveAspectFit
            source: root.__thumbPath
            sourceSize.width: 300 * uiScale
            sourceSize.height: 300 * uiScale
        }
    }
    Component.onCompleted: {
        loadThumb.sourceComponent = thumbImage
    }

}
