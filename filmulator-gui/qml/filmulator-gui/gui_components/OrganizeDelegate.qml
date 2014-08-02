import QtQuick 2.2

Rectangle {
    id: root
    width: 300
    height: 300
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
            sourceSize.width: 300
            sourceSize.height: 300
        }
    }
    Component.onCompleted: {
        loadThumb.sourceComponent = thumbImage
    }

}
