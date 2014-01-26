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
        color: "black"
        Layout.fillWidth: true
        Flickable {
            id: flicky
            x: 0
            y: 30
            width: parent.width
            height: parent.height-30
            contentWidth: Math.max(largeview1.width*largeview1.scale,this.width);
            contentHeight: Math.max(largeview1.height*largeview1.scale,this.height);
            flickableDirection: Flickable.HorizontalAndVerticalFlick
            clip: true
            Rectangle {
                id: largeview
                //scale: largeview1.scale
                width: Math.max(largeview1.width*largeview1.scale,parent.width);
                height: Math.max(largeview1.height*largeview1.scale,parent.height);
                //property real aspectRatio: (this.height != 0) ? this.width / this.height : 0
                transformOrigin: Item.TopLeft
                color: "#000000"
                Image {
                    anchors.centerIn: parent
                    id: largeview1
                    source:"image://filmy/"+location+index
                    fillMode: Image.PreserveAspectFit
                    asynchronous: true
                    //property real aspectRatio: (this.sourceSize.height != 0) ? this.sourceSize.width/this.sourceSize.height : 0
                    onStatusChanged: if(largeview1.status == Image.Ready) {
                                         //this.aspectRatio = (this.sourceSize.height != 0) ? this.sourceSize.width/this.sourceSize.height : 0
                                         //if(Math.abs(largeview.aspectRatio-this.aspectRatio) > .05 ) {
                                         //largeview.width = this.sourceSize.width
                                         //largeview.height = this.sourceSize.height
                                         largeview2.source = largeview1.source
                                     }
                }
                Image {
                    anchors.centerIn: parent
                    id: largeview2
                    scale: largeview1.scale
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
                    largeview1.scale = 1
                    //largeview.width = largeview1.sourceSize.width
                    //largeview.height = largeview1.sourceSize.height
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
                    largeview1.scale *= 1.2
                    //largeview.width *= 1.3;
                    //largeview.height *= 1.3;
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
                    largeview1.scale /= 1.2;
                    //largeview.width = largeview.width/1.3;
                    //largeview.height = largeview.height/1.3;
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
                text: qsTr("contentwidth: ")+flicky.contentWidth+qsTr(" contentheight: ")+flicky.contentHeight
                color: "white"
            }
        }
    }
    Rectangle {
        id: tools
        color: "brown"
        width: 150
        Layout.maximumWidth: 500
        Flickable {
            flickableDirection: Qt.Horizontal
            anchors.fill: parent
            Slider {
                width: parent.width
                height: 30
                minimumValue: -5
                maximumValue: 5
                stepSize: 1/3
                value: 0
                onValueChanged: {
                    filmProvider.setExposureComp(value)
                    filmProvider.invalidateFilmulation();
                    editortab.rolling = (editortab.rolling + 1)%10
                }
                updateValueWhileDragging: true
            }
            Slider {
                width: parent.width
                height: 30
                y: parent.y + 20
                minimumValue: 0
                maximumValue: 5/1000
                stepSize: 0.1/1000
                value: 2/1000
                onValueChanged: {
                    filmProvider.setWhitepoint(value)
                    editortab.rolling = (editortab.rolling + 1)%10
                }
                updateValueWhileDragging: true
            }
        }
    }
}
