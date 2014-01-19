import QtQuick 2.2
import QtQuick.Controls 1.1
import QtQuick.Layouts 1.1

SplitView {
    anchors.fill: parent
    orientation: Qt.Horizontal
    property string location

    Rectangle {
        id: photobox
        color: "green"
        Layout.fillWidth: true
        Flickable {
            x: 0
            y: 30
            width: parent.width
            height: parent.height-30
            contentWidth: largeview.width
            contentHeight: largeview.height
            flickableDirection: Flickable.HorizontalAndVerticalFlick
            clip: true

            Image {
                id: largeview
                source:"image://filmy/"+location
                fillMode: Image.PreserveAspectFit
            }
        }
        Button {
            id: fullzoom
            width: 30
            height: 30
            x: parent.width-90
            y: 0
            text: "1:1"
            action: Action {
                onTriggered: {
                    largeview.width = largeview.sourceSize.width
                    largeview.height = largeview.sourceSize.height
                }
            }
        }

        Button {
            id: zoomin
            width: 30
            height: 30
            x: parent.width-60
            y: 0
            text: "+"
            action: Action {
                onTriggered: {
                    largeview.width *= 2;
                    largeview.height *= 2;
                }
            }
        }
        Button {
            id: zoomout
            width:30
            height:30
            x: parent.width-30
            y: 0
            text: "-"
            action: Action {
                onTriggered: {
                    largeview.width = largeview.width*0.5;
                    largeview.height = largeview.height*0.5;
                }
            }
        }
        Rectangle {
            id: dims
            width: 300
            height: 30
            x: 0
            y: 0
            color: "black"
            Text {
                text: "width: " + largeview.width + " height: " + largeview.height
                color: "white"
            }
        }
    }
    Rectangle {
        id: tools
        color: "brown"
        width: 150
        Layout.maximumWidth: 500
    }
}
