import QtQuick 2.2
import QtQuick.Controls 1.1
import QtQuick.Layouts 1.1
import QtWebKit 3.0
import "gui_components"

SplitView {
    anchors.fill: parent
    orientation: Qt.Horizontal
    property string location
    property int index
    property real exposureComp
    property real whitepoint

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
            contentWidth: Math.max(largeview1.width*largeview1.scale,this.width);
            contentHeight: Math.max(largeview1.height*largeview1.scale,this.height);
            flickableDirection: Flickable.HorizontalAndVerticalFlick
            clip: true
            property real fitScaleX: flicky.width/largeview1.width
            property real fitScaleY: flicky.height/largeview1.height
            property real fitScale: Math.min(fitScaleX, fitScaleY)

            //The centers are the coordinates in display space of the center of the image.
            property real centerX: (contentX + largeview1.width*Math.min(largeview1.scale,fitScaleX)/2)/largeview1.scale
            property real centerY: (contentY + largeview1.height*Math.min(largeview1.scale,fitScaleY)/2)/largeview1.scale
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
                    property real realWidth: width * scale
                    property real realHeight: height * scale
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
        MouseArea {
            id: wheelCapture
            anchors.fill: flicky
            acceptedButtons: Qt.NoButton
            onWheel: {
                //We recognize that flicky and wheelCapture have the same size and position, thus
                // mouse has the same coordinates in both.
                //var mouseX = (flicky.contentX + point.x*Math.min(largeview1.scale, flicky.fitScaleX))/largeview1.scale
                //var mouseY = (flicky.contentY + point.x*Math.min(largeview1.scale, flicky.fitScaleX))/largeview1.scale
                var factorX
                var factorY
                var oldContentX = flicky.contentX
                var oldContentY = flicky.contentY
                if (wheel.angleDelta.y > 0) {
                    factorX = Math.min(0.2, Math.max((flicky.fitScaleX/largeview1.scale)-1, 0.0));
                    factorY = Math.min(0.2, Math.max((flicky.fitScaleY/largeview1.scale)-1, 0.0));
                    largeview1.scale *= 1.2;
                    flicky.contentX = oldContentX*1.2 + wheel.x*0.2 - wheelCapture.width/2*factorX;
                    flicky.contentY = oldContentY*1.2 + wheel.y*0.2 - wheelCapture.height/2*factorY;
                }
                else {
                    factorX = Math.max(-1/6, Math.min((largeview1.scale/(1.2*flicky.fitScaleX))-1, 0));
                    factorY = Math.max(-1/6, Math.min((largeview1.scale/(1.2*flicky.fitScaleY))-1, 0));
                    largeview1.scale /= 1.2;
                    flicky.contentX = oldContentX/1.2 - wheel.x/6 - wheelCapture.width/2*factorX;
                    flicky.contentY = oldContentY/1.2 - wheel.y/6 - wheelCapture.height/2*factorY;
                    flicky.returnToBounds()
                }
            }
        }

        Button {
            id: fitscreen
            width: 30
            height: 30
            x: parent.width-120
            y: 0
            text: qsTr("Fit")
            action: Action {
                onTriggered: {
                    if(largeview1.width != 0 && largeview1.height != 0) {
                        largeview1.scale = flicky.fitScale
                    }
                    else {
                        largeview1.scale = 1
                    }
                    flicky.returnToBounds()
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
                    var oldCenterX = flicky.centerX;
                    var oldCenterY = flicky.centerY;
                    largeview1.scale = 1;
                    flicky.contentX = oldCenterX*1 - largeview1.width*Math.min(1,flicky.fitScaleX)/2;
                    flicky.contentY = oldCenterY*1 - largeview1.height*Math.min(1,flicky.fitScaleY)/2;
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
                    var oldCenterX = flicky.centerX;
                    var oldCenterY = flicky.centerY;
                    largeview1.scale *= 1.2
                    flicky.contentX = oldCenterX*largeview1.scale - largeview1.width*Math.min(largeview1.scale,flicky.fitScaleX)/2;
                    flicky.contentY = oldCenterY*largeview1.scale - largeview1.height*Math.min(largeview1.scale,flicky.fitScaleY)/2;
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
                    var oldCenterX = flicky.centerX;
                    var oldCenterY = flicky.centerY;
                    largeview1.scale /= 1.2;
                    var tempScale = Math.max(largeview1.scale, flicky.fitScale)
                    flicky.contentX = oldCenterX*tempScale - largeview1.width*Math.min(tempScale,flicky.fitScaleX)/2;
                    flicky.contentY = oldCenterY*tempScale - largeview1.height*Math.min(tempScale,flicky.fitScaleY)/2;
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
    Rectangle {
        id: tools
        color: "#404040"
        width: 250
        Layout.maximumWidth: 500
        Layout.minimumWidth: 250
        Flickable {
            flickableDirection: Qt.Vertical
            anchors.fill: parent
            Column {
                spacing: 10
                anchors.fill: parent

                Canvas {
                    id:canvas
                    width:parent.width
                    height:100
                    property int lineWidth: 1
                    property real alpha: 1.0
                    property int hist: filmProvider.hist
                    antialiasing: true

                    onWidthChanged:requestPaint();
                    onHistChanged: requestPaint();

                    onPaint: {
                        var ctx = canvas.getContext('2d');
                        ctx.save();
                        ctx.clearRect(0, 0, canvas.width, canvas.height);
                        ctx.globalAlpha = canvas.alpha;
                        ctx.lineWidth = canvas.lineWidth;
                        var myGradient = ctx.createLinearGradient(0,0,canvas.width,0);
                        var hist = canvas.hist;

                        var startx = 10;
                        var endx = canvas.width - 10;
                        var graphwidth = endx - startx;
                        var starty = canvas.height - 10;
                        var endy = 10;
                        var graphheight = starty - endy;
                        var histPoint = 0;
                        var maxValue = 128.0

                        //rCurve
                        ctx.beginPath()
                        ctx.moveTo(startx,starty);
                        for(var i = 0; i < maxValue; i++)
                        {
                            histPoint = filmProvider.getRHistogramPoint(i);
                            ctx.lineTo(startx+(i/maxValue)*graphwidth,starty-(histPoint)*graphheight);
                        }
                        ctx.lineTo(endx,starty);
                        ctx.lineTo(startx,starty);
                        ctx.closePath();
                        myGradient.addColorStop(1,"red");
                        myGradient.addColorStop(0,'rgb(180,0,0)');
                        ctx.fillStyle = myGradient;
                        ctx.fill();

                        //gCurve
                        ctx.beginPath()
                        ctx.moveTo(startx,starty);
                        for(var i = 0; i < maxValue; i++)
                        {
                            histPoint = filmProvider.getGHistogramPoint(i);
                            ctx.lineTo(startx+(i/maxValue)*graphwidth,starty-(histPoint)*graphheight);
                        }
                        ctx.lineTo(endx,starty);
                        ctx.lineTo(startx,starty);
                        ctx.closePath();
                        myGradient.addColorStop(1,"green");
                        myGradient.addColorStop(0,'rgb(0,180,0)');
                        ctx.fillStyle = myGradient;
                        ctx.fill();

                        //bCurve
                        ctx.beginPath()
                        ctx.moveTo(startx,starty);
                        for(var i = 0; i < maxValue; i++)
                        {
                            histPoint = filmProvider.getBHistogramPoint(i);
                            ctx.lineTo(startx+(i/maxValue)*graphwidth,starty-(histPoint)*graphheight);
                        }
                        ctx.lineTo(endx,starty);
                        ctx.lineTo(startx,starty);
                        ctx.closePath();
                        myGradient.addColorStop(1,"blue");
                        myGradient.addColorStop(0,'rgb(0,0,180)');
                        ctx.fillStyle = myGradient;
                        ctx.fill();

                        //Luma curve
                        ctx.beginPath();
                        ctx.moveTo(startx,starty);
                        for(var i = 0; i < maxValue; i++)
                        {
                            histPoint = filmProvider.getLumaHistogramPoint(i);
                            ctx.lineTo(startx+(i/maxValue)*graphwidth,starty-(histPoint)*graphheight);
                        }
                        ctx.lineTo(endx,starty);
                        ctx.lineTo(startx,starty);
                        ctx.closePath();
                        myGradient.addColorStop(1,"white");
                        myGradient.addColorStop(0,'rgb(180,180,180)');
                        ctx.fillStyle = myGradient;
                        ctx.fill()

                        ctx.strokeStyle = "#000000";
                        ctx.strokeRect(startx,endy,graphwidth,graphheight);

                        ctx.restore();
                    }
                }


                RowLayout{
                    width: parent.width
                    spacing: 10
                    Text{
                        text: "Exposure Comp"
                    }
                    Slider {
                        id: exposureCompSlider
                        width: parent.width
                        height: 30
                        minimumValue: -5
                        maximumValue: 5
                        stepSize: 1/3
                        value: exposureComp
                        onValueChanged: {
                            filmProvider.exposureComp = value
                            editortab.rolling = (editortab.rolling + 1)%10
                        }
                        updateValueWhileDragging: true
                        Layout.fillWidth: true
                    }
                    Text{
                        text: exposureComp
                    }
                }

                RowLayout{
                    spacing: 10

                    Text{
                        text:"White"
                    }
                    Slider {
                        id: whitepointSlider
                        width: parent.width
                        height: 30
                        //y: parent.y + 20
                        minimumValue: 0.1/1000
                        maximumValue: 5/1000
                        stepSize: 0.1/1000
                        value: whitepoint
                        onValueChanged: {
                            filmProvider.whitepoint = (5.1/1000) - value
                            editortab.rolling = (editortab.rolling + 1)%10
                        }
                        updateValueWhileDragging: true
                        Layout.fillWidth: true
                    }
                    Text{
                        text:whitepoint
                    }
                }

                ToolSlider {
                    id: hi
                    title: qsTr("Tool a")
                    minimumValue: -10
                    maximumValue: 10
                    stepSize: 1
                    value: 0
                    //y: parent.y+40
                }
            }
        }
    }
}
