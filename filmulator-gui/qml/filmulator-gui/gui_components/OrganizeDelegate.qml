import QtQuick 2.2

Rectangle {
    id: root
    width: 300
    height: 300
    color: "#00000000"
    property string rootDir
    property string searchID

    property string __thumbPath: rootDir + '/'+ searchID.slice(0,4) + '/' + searchID + '.jpg'


    Rectangle {
        anchors.fill: parent
        color: "green"
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
        }
    }
    Component.onCompleted: {
        loadThumb.sourceComponent = thumbImage
    }

}
