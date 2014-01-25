import QtQuick 2.2
import QtQuick.Controls 1.1
import QtQuick.Layouts 1.1

SplitView {
    anchors.fill: parent
    orientation: Qt.Horizontal
    property string location
    property int index

    Rectangle {
        id: photobox
        color: "green"
        Layout.fillWidth: true
        Flickable {
            x: 0
            y: 30
            width: parent.width
            height: parent.height-30
            contentWidth: largeview1.width
            contentHeight: largeview1.height
            flickableDirection: Flickable.HorizontalAndVerticalFlick
            clip: true
            Rectangle {
                id: largeview
                property real aspectRatio: (this.height != 0) ? this.width / this.height : 0
                Image {
                    anchors.fill: parent
                    id: largeview1
                    source:"image://filmy/"+location+index
                    fillMode: Image.PreserveAspectFit
                    asynchronous: true
                    property real aspectRatio: (this.sourceSize.height != 0) ? this.sourceSize.width/this.sourceSize.height : 0
                    onStatusChanged: if(largeview1.status == Image.Ready) {
                                         this.aspectRatio = (this.sourceSize.height != 0) ? this.sourceSize.width/this.sourceSize.height : 0
                                         if(Math.abs(largeview.aspectRatio-this.aspectRatio) > .05 ) {
                                             largeview.width = this.sourceSize.width
                                             largeview.height = this.sourceSize.height
                                         }
                                         largeview2.source = largeview1.source
                                     }
                }
                Image {
                    anchors.fill: parent
                    id: largeview2
                    fillMode: Image.PreserveAspectFit
                    asynchronous: true
                }
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
                    largeview.width = largeview1.sourceSize.width
                    largeview.height = largeview1.sourceSize.height
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
                    largeview.width *= 1.3;
                    largeview.height *= 1.3;
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
                    largeview.width = largeview.width/1.3;
                    largeview.height = largeview.height/1.3;
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
                text: qsTr("width: ") + largeview.width + qsTr(" height: ") + largeview.height
                color: "white"
            }
            Text {
                y: 15
                text: qsTr("outeraspect: " ) + largeview.aspectRatio + qsTr(" imageaspect: " ) + largeview1.aspectRatio
                color: "white"
            }
        }
    }
    Rectangle {
        id: tools
        color: "brown"
        width: 150
        Layout.maximumWidth: 500

        Slider {
            width: parent.width
            height: 30
            minimumValue: -5
            maximumValue: 5
            stepSize: 1/3
            value: 0
            onValueChanged: {
                filmProvider.setExposureComp(value)
                editortab.rolling = (editortab.rolling + 1)%10
            }
            updateValueWhileDragging: true
        }
    }
}
