import QtQuick 2.2
import QtQuick.Controls 1.1
import QtQuick.Layouts 1.1
import QtWebKit 3.0
import "gui_components"

SplitView {
    id: editorsplit
    anchors.fill: parent
    orientation: Qt.Horizontal
    property string location
    property int index
    property real exposureComp
    property real whitepoint
    property real blackpoint
    property real shadowY
    property real highlightY
    property real filmSize
    property bool defaultCurve

    onLocationChanged: filmProvider.invalidateImage()

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
            contentWidth: Math.max(largeview2.width*largeview2.scale,this.width);
            contentHeight: Math.max(largeview2.height*largeview2.scale,this.height);
            flickableDirection: Flickable.HorizontalAndVerticalFlick
            clip: true
            property real fitScaleX: flicky.width/largeview2.width
            property real fitScaleY: flicky.height/largeview2.height
            property real fitScale: Math.min(fitScaleX, fitScaleY)

            //The centers are the coordinates in display space of the center of the image.
            property real centerX: (contentX + largeview2.width*Math.min(largeview2.scale,fitScaleX)/2)/largeview2.scale
            property real centerY: (contentY + largeview2.height*Math.min(largeview2.scale,fitScaleY)/2)/largeview2.scale
            Rectangle {
                id: largeview
                //scale: largeview1.scale
                width: Math.max(largeview2.width*largeview2.scale,parent.width);
                height: Math.max(largeview2.height*largeview2.scale,parent.height);
                //property real aspectRatio: (this.height != 0) ? this.width / this.height : 0
                transformOrigin: Item.TopLeft
                color: "#000000"
                Image {
                    anchors.centerIn: parent
                    id: largeview1
                    source:"image://filmy/"+location+index
                    fillMode: Image.PreserveAspectFit
                    asynchronous: true
                    layer.mipmap: true
                    property real realWidth: width * scale
                    property real realHeight: height * scale
                    scale: largeview2.scale
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
                    fillMode: Image.PreserveAspectFit
                    asynchronous: true
                    layer.mipmap: true
                }
            }
        }
        MouseArea {
            id: wheelCapture
            anchors.fill: flicky
            acceptedButtons: Qt.RightButton
            onDoubleClicked: {
                if(largeview2.scale < flicky.fitScale || largeview2.scale == 1) {
                    largeview2.scale = flicky.fitScale
                    flicky.returnToBounds()
                }
                else {
                    var zoomFactor = 1/largeview2.scale

                    var oldMouseX = mouse.x + flicky.contentX - Math.max(0, 0.5*(flicky.width-largeview2.width*largeview2.scale))
                    var oldMouseY = mouse.y + flicky.contentY - Math.max(0, 0.5*(flicky.height-largeview2.height*largeview2.scale))

                    largeview2.scale = 1

                    //for the following, the last largeview2.scale is now 1, so we just leave it off.
                    flicky.contentX = oldMouseX*zoomFactor - mouse.x + Math.max(0,0.5*(flicky.width-largeview2.width))
                    flicky.contentY = oldMouseY*zoomFactor - mouse.y + Math.max(0,0.5*(flicky.height-largeview2.height))

                    flicky.returnToBounds()
                }
            }

            onWheel: {
                var oldMouseX = wheel.x + flicky.contentX - Math.max(0, 0.5*(flicky.width-largeview2.width*largeview2.scale))
                var oldMouseY = wheel.y + flicky.contentY - Math.max(0, 0.5*(flicky.height-largeview2.height*largeview2.scale))
                var zoomFactor = 1.2
                if (wheel.angleDelta.y > 0) {
                    largeview2.scale *= zoomFactor;
                    flicky.contentX = oldMouseX*zoomFactor - wheel.x + Math.max(0,0.5*(flicky.width-largeview2.width*largeview2.scale))
                    flicky.contentY = oldMouseY*zoomFactor - wheel.y + Math.max(0,0.5*(flicky.height-largeview2.height*largeview2.scale))
                }
                else {
                    largeview2.scale /= zoomFactor;
                    flicky.contentX = oldMouseX/zoomFactor - wheel.x + Math.max(0,0.5*(flicky.width-largeview2.width*largeview2.scale))
                    flicky.contentY = oldMouseY/zoomFactor - wheel.y + Math.max(0,0.5*(flicky.height-largeview2.height*largeview2.scale))
                    flicky.returnToBounds()
                }
            }
        }

        ToolButton {
            id: fitscreen
            //width: 30
            //height: 30
            x: parent.width-120
            y: 0
            text: qsTr("Fit")
            action: Action {
                onTriggered: {
                    if(largeview2.width != 0 && largeview2.height != 0) {
                        largeview2.scale = flicky.fitScale
                    }
                    else {
                        largeview2.scale = 1
                    }
                    flicky.returnToBounds()
                }
            }
        }

        ToolButton {
            id: fullzoom
            //width: 30
            //height: 30
            x: parent.width-90
            y: 0
            text: "1:1"
            action: Action {
                onTriggered: {
                    var oldCenterX = flicky.centerX;
                    var oldCenterY = flicky.centerY;
                    largeview2.scale = 1;
                    flicky.contentX = oldCenterX*1 - largeview2.width*Math.min(1,flicky.fitScaleX)/2;
                    flicky.contentY = oldCenterY*1 - largeview2.height*Math.min(1,flicky.fitScaleY)/2;
                }
            }
        }

        ToolButton {
            id: zoomin
            //width: 30
            //height: 30
            x: parent.width-60
            y: 0
            text: "+"
            action: Action {
                onTriggered: {
                    var oldCenterX = flicky.centerX;
                    var oldCenterY = flicky.centerY;
                    largeview2.scale *= 1.2
                    flicky.contentX = oldCenterX*largeview2.scale - largeview2.width*Math.min(largeview2.scale,flicky.fitScaleX)/2;
                    flicky.contentY = oldCenterY*largeview2.scale - largeview2.height*Math.min(largeview2.scale,flicky.fitScaleY)/2;
                }
            }
        }
        ToolButton {
            id: zoomout
            //width:30
            //height:30
            x: parent.width-30
            y: 0
            text: "-"
            action: Action {
                onTriggered: {
                    var oldCenterX = flicky.centerX;
                    var oldCenterY = flicky.centerY;
                    largeview2.scale /= 1.2;
                    var tempScale = Math.max(largeview2.scale, flicky.fitScale)
                    flicky.contentX = oldCenterX*tempScale - largeview2.width*Math.min(tempScale,flicky.fitScaleX)/2;
                    flicky.contentY = oldCenterY*tempScale - largeview2.height*Math.min(tempScale,flicky.fitScaleY)/2;
                    flicky.returnToBounds();
                }
            }
        }
        ProgressBar {
            indeterminate: false;
            orientation: Qt.Horizontal;
            visible: true;
            value: filmProvider.progress;
        }
        Text {
            id: text1
            x: 200
            y: 0
            color: "white"
            text: flicky.contentX - (flicky.width-largeview1.width*largeview1.scale)/2
        }
        Text{
            id: text2
            x: 200
            y: 15
            color: "white"
            text: flicky.contentX
        }
    }
    EditTools {
        index: index
        exposureComp: editorsplit.exposureComp
        whitepoint: editorsplit.whitepoint
        blackpoint: editorsplit.blackpoint
        shadowY: editorsplit.shadowY
        highlightY: editorsplit.highlightY
        filmSize: editorsplit.filmSize
        defaultCurve: editorsplit.defaultCurve

    }
}
