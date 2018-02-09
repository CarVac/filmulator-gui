import QtQuick 2.3
import QtQuick.Controls 1.2
import QtQuick.Layouts 1.1
import "gui_components"

SplitView {
    id: root
    anchors.fill: parent
    orientation: Qt.Horizontal
    property real uiScale: 1
    property bool imageReady: false//must only be made ready when the full size image is ready
    property bool requestingCropping: false
    property bool cropping: false
    property bool cancelCropping: false
    property real cropMargin: 50//200

    onRequestingCroppingChanged: {
        if (requestingCropping == true) {
            if (paramManager.cropHeight <= 0) {//No crop in the database
                //load default params
                imageRect.cropHeight = 1
                imageRect.cropAspect = bottomImage.width/bottomImage.height
                imageRect.cropVoffset = 0
                imageRect.cropHoffset = 0
                //The image probably won't update, so we need to manually turn on cropping.
                cropping = true
            } else {//there was already a crop
                //load from database
                imageRect.cropHeight = paramManager.cropHeight
                imageRect.cropAspect = paramManager.cropAspect
                imageRect.cropVoffset = paramManager.cropVoffset
                imageRect.cropHoffset = paramManager.cropHoffset
                paramManager.cropHeight = 0
            }
        } else {//we're done cropping
            if (!cancelCropping) {//accepted the crop
                //send stuff back to database
                paramManager.cropHeight = imageRect.readHeight
                paramManager.cropAspect = imageRect.readAspect
                paramManager.cropVoffset = imageRect.readVoffset
                paramManager.cropHoffset = imageRect.readHoffset
                paramManager.writeback()
            } else {
                cancelCropping = false
            }
        }
    }

    onCroppingChanged: {
        if (flicky.fit) {
            bottomImage.scale = flicky.fitScale
        }
        flicky.returnToBounds()
        flicky.contentX = flicky.contentX + 2*Math.floor(cropMargin*uiScale*cropping) - Math.floor(cropMargin*uiScale)
        flicky.contentY = flicky.contentY + 2*Math.floor(cropMargin*uiScale*cropping) - Math.floor(cropMargin*uiScale)
    }
    signal tooltipWanted(string text, int x, int y)

    //This is for telling the queue the latest image source so it can show it until the thumb updates.
    signal imageURL(string newURL)


    Rectangle {
        id: photoBox
        color: "black"
        Layout.fillWidth: true
        property int backgroundColor: backgroundBrightnessSlider.value
        Rectangle {//This is because before the image is loaded there's no background.
            id: background
            x: 0 * uiScale
            y: Math.ceil(30 * uiScale)
            width: parent.width
            height: Math.floor(parent.height - 30 * uiScale)
            color: photoBox.backgroundColor == 2 ? "white" : photoBox.backgroundColor == 1 ? "gray" : "black"
        }

        Flickable {
            id: flicky
            x: 0 * uiScale
            y: Math.ceil(30 * uiScale)
            width: parent.width
            height: Math.floor(parent.height - 30 * uiScale)
            contentWidth:  Math.max(bottomImage.width *bottomImage.scale, this.width) +2*Math.floor(cropMargin*uiScale*cropping);
            contentHeight: Math.max(bottomImage.height*bottomImage.scale, this.height)+2*Math.floor(cropMargin*uiScale*cropping);
            flickableDirection: Flickable.HorizontalAndVerticalFlick
            clip: true
            pixelAligned: true
            property real fitScaleX: flicky.width/bottomImage.width
            property real fitScaleY: flicky.height/bottomImage.height
            property real fitScale: Math.min(fitScaleX, fitScaleY)
            property real sizeRatio: 1
            property bool fit: true
            //Here, if the window size changed, we set it to fitScale. Except that it didn't update in time, so we make it compute it from scratch.
            onWidthChanged:  if (flicky.fit) {bottomImage.scale = Math.min(flicky.width/bottomImage.width, flicky.height/bottomImage.height)}
            onHeightChanged: if (flicky.fit) {bottomImage.scale = Math.min(flicky.width/bottomImage.width, flicky.height/bottomImage.height)}

            //The centers are the coordinates in display space of the center of the image.
            //They're used for the zoom buttons which zoom about the center of the screen.
            property real centerX: (contentX +  bottomImage.width*Math.min(bottomImage.scale, fitScaleX)/2) / bottomImage.scale
            property real centerY: (contentY + bottomImage.height*Math.min(bottomImage.scale, fitScaleY)/2) / bottomImage.scale
            Rectangle {
                id: imageRect
                //The dimensions here need to be floor because it was yielding non-pixel widths.
                //That caused the child images to be offset by fractional pixels at 1:1 scale when the
                // image is smaller than the flickable in one or more directions.
                x: -Math.floor(cropMargin*uiScale*cropping)
                y: -Math.floor(cropMargin*uiScale*cropping)
                width: Math.floor(Math.max(bottomImage.width*bottomImage.scale,parent.width)) + 2*Math.floor(cropMargin*uiScale*cropping)
                height: Math.floor(Math.max(bottomImage.height*bottomImage.scale,parent.height)) + 2*Math.floor(cropMargin*uiScale*cropping)
                transformOrigin: Item.TopLeft
                color: photoBox.backgroundColor == 2 ? "white" : photoBox.backgroundColor == 1 ? "gray" : "black"
                Image {
                    x: Math.floor(parent.width/2) - Math.floor(width*scale/2)
                    y: Math.floor(parent.height/2) - Math.floor(height*scale/2)
                    id: topImage
                    fillMode: Image.PreserveAspectFit
                    asynchronous: true
                    mipmap: settings.getMipmapView()
                    property int index: 0
                    property string indexString: "000000"
                    scale: bottomImage.scale
                    transformOrigin: Item.TopLeft

                    //This is a hidden image to do filmImageProvider loading without interrupting thumbnails.
                    Image {
                        z: -1
                        id: hiddenImage
                        x:0
                        y:0
                        mipmap: settings.getMipmapView()
                        opacity: 0
                        asynchronous: true
                        onStatusChanged: {
                            if (hiddenImage.status == Image.Ready) {
                                console.log("hidden image ready; state: " + topImage.state)
                                //topImage.state = "i"//probably unnecessary?
                                topImage.source = hiddenImage.source

                                if ("i" == topImage.state) {
                                    //do nothing
                                }
                                else {
                                    root.imageReady = false
                                }
                            }
                            else {
                                root.imageReady = false
                            }
                        }
                    }

                    property string state: "p"
                    //This connection finds out when the parameters are done changing, and then updates the url to fetch the latest image.
                    Connections {
                        target: paramManager//root
                        onUpdateImage: {
                            console.log("update image")
                            if (topImage.state == "i" || topImage.state == "q") {//only if we're done with the thumbnail
                                if (settings.getQuickPreview()) {
                                    topImage.state = "q"//make it do the quick processed image first
                                }//otherwise, leave it as whatever it was.

                                var num = (topImage.index + 1) % 1000000//1 in a million
                                topImage.index = num;
                                var s = num+"";
                                var size = 6 //6 digit number
                                while (s.length < size) {s = "0" + s}
                                topImage.indexString = s
                                hiddenImage.source = "image://filmy/q" + topImage.indexString
                            }
                            if (topImage.state == "p") {//if we're planning on doing the thumbnail
                                var thumbPath = organizeModel.thumbDir() + '/' + paramManager.imageIndex.slice(0,4) + '/' + paramManager.imageIndex + '.jpg'
                                topImage.source = thumbPath
                            }
                            //if a new image is selected, OR the same image re-selected
                            if (newImage) {
                                cancelCropping = true
                                requestingCropping = false
                            }
                        }
                        onImageIndexChanged: {
                            console.log("image index changed")
                            topImage.state = "p"
                            //topImage.source = topImage.thumbPath
                        }
                    }
                    Connections {
                        target: settings
                        onMipmapViewChanged: topImage.mipmap = settings.getMipmapView()
                    }
                    onStatusChanged: {
                        if (topImage.status == Image.Ready) {
                            console.log("top image ready")
                            var topFitScaleX = flicky.width/topImage.width
                            var topFitScaleY = flicky.height/topImage.height
                            var topFitScale = Math.min(topFitScaleX, topFitScaleY)
                            flicky.sizeRatio = topFitScale / flicky.fitScale
                            if (flicky.sizeRatio > 1) {
                                //If the new image is smaller, we need to scale the bottom image up before selecting the image.
                                bottomImage.scale = bottomImage.scale * flicky.sizeRatio
                                bottomImage.source = topImage.source
                            }
                            else {
                                //If the new image is bigger, we want to select the image and then scale it down.
                                bottomImage.source = topImage.source
                                //This has to happen after the size actually changes. It's put below.
                            }

                            console.log("TopImage state: " + topImage.state)

                            if (topImage.state == "i") {//if it's the full image
                                //now we notify the queue that the latest image is ready for display
                                root.imageReady = true
                                root.imageURL(topImage.source)
                            }
                            else if (topImage.state == "q") {//if the Quick processed image is loaded
                                topImage.state = "i"
                                console.log("TopImage state after quick: " + topImage.state)

                                //begin loading the main, full-size image image
                                var num = (topImage.index + 1) % 1000000//1 in a million
                                topImage.index = num;
                                var s = num+"";
                                var size = 6 //6 digit number
                                while (s.length < size) {s = "0" + s}
                                topImage.indexString = s
                                hiddenImage.source = "image://filmy/i" + topImage.indexString
                            }
                            else if (topImage.state == "p") {//if the thumbnail Preview is loaded
                                //now we say that the state is loading the actual image, so as to not load a new preview.
                                if (settings.getQuickPreview()) {
                                    topImage.state = "q"
                                }
                                else {//skip the quick preview
                                    topImage.state = "i"
                                }
                                console.log("TopImage state after thumb: " + topImage.state)

                                //begin loading the image
                                var num = (topImage.index + 1) % 1000000//1 in a million
                                topImage.index = num;
                                var s = num+"";
                                var size = 6 //6 digit number
                                while (s.length < size) {s = "0" + s}
                                topImage.indexString = s
                                hiddenImage.source = "image://filmy/q" + topImage.indexString
                            }
                            if (root.requestingCropping) {
                                root.cropping = true
                            } else {
                                root.cropping = false
                            }
                        }
                    }
                }
                Image {
                    x: Math.floor(parent.width/2) - Math.floor(width*scale/2)
                    y: Math.floor(parent.height/2) - Math.floor(height*scale/2)
                    id: bottomImage
                    fillMode: Image.PreserveAspectFit
                    asynchronous: true
                    mipmap: settings.getMipmapView()
                    transformOrigin: Item.TopLeft
                    onStatusChanged: {
                        if (bottomImage.status == Image.Ready) {
                            if (flicky.fit) {
                                //This is probably not necessary, but I don't want rounding errors to crop up.
                                bottomImage.scale = flicky.fitScale
                            }
                            else if (flicky.sizeRatio <= 1) {
                                bottomImage.scale = bottomImage.scale * flicky.sizeRatio
                            }
                        }
                    }
                    Connections {
                        target: settings
                        onMipmapViewChanged: bottomImage.mipmap = settings.getMipmapView()
                    }
                }
                MouseArea {
                    id: doubleClickCapture
                    anchors.fill: parent
                    hoverEnabled: true
                    acceptedButtons: Qt.LeftButton
                    onDoubleClicked: {
                        if (bottomImage.scale < flicky.fitScale || bottomImage.scale == 1) {
                            bottomImage.scale = flicky.fitScale
                            flicky.contentX = Math.floor(cropMargin*uiScale*cropping)
                            flicky.contentY = Math.floor(cropMargin*uiScale*cropping)
                            flicky.fit = true
                        } else {
                            var zoomFactor = 1/bottomImage.scale

                            //Here's how it worked before the cropmargin was added
                            //oldContentX = flicky.contentX

                            //            distance from edge of imageRect to mouse cursor, no scaling
                            //                      space to the left of the image
                            //basically, the coordinates are relative to the edge of the image.
                            //oldMouseX = mouse.x - Math.max(0, 0.5*(flicky.width  - bottomImage.width*bottomImage.scale))

                            //                  changes are relative to the previous contentX
                            //                                when you zoom in, the distance between the mouse and the edge of the image must increase by the zoomfactor
                            //                                                       but only by the zoomfactor, so we subtract a recalculation of oldMouseX; recalculated because the of the margins changing.
                            //                                                                                                  not multiplied by bottomImage.scale since that's now 1
                            //flicky.contentX = oldContentX + oldMouseX*zoomFactor - mouse.x + Math.max(0, 0.5*(flicky.width  - bottomImage.width))

                            //The contentX still stays the same. We're just saving it for after the scale changes.
                            var oldContentX = flicky.contentX
                            var oldContentY = flicky.contentY

                            //                                                                                                  2 because the imageRect is 1 above and to the left of where it should be
                            var oldMouseX = mouse.x - Math.max(0, 0.5*(flicky.width  - bottomImage.width*bottomImage.scale))  - 2*Math.floor(cropMargin*uiScale*cropping)
                            var oldMouseY = mouse.y - Math.max(0, 0.5*(flicky.height - bottomImage.height*bottomImage.scale)) - 2*Math.floor(cropMargin*uiScale*cropping)

                            bottomImage.scale = 1

                            var newMouseX = mouse.x - Math.max(0, 0.5*(flicky.width  - bottomImage.width))  - 2*Math.floor(cropMargin*uiScale*cropping)
                            var newMouseY = mouse.y - Math.max(0, 0.5*(flicky.height - bottomImage.height)) - 2*Math.floor(cropMargin*uiScale*cropping)

                            flicky.contentX = oldContentX + oldMouseX*zoomFactor - newMouseX
                            flicky.contentY = oldContentY + oldMouseY*zoomFactor - newMouseY

                            flicky.returnToBounds()
                            if (bottomImage.scale == flicky.fitScale) {flicky.fit = true}
                            else {flicky.fit = false}
                        }
                    }
                }

                //From here are the crop markers.
                //There are four parameters that get stored.
                // crop height as % of image height
                //property real cropHeight: 0.5312353
                property real cropHeight: 0//0.5225
                // width / height (aspect ratio of the crop)
                property real cropAspect: 0//1.5
                // voffset as % of image height, center from center
                property real cropVoffset: 0//0.0
                // hoffset as % of image width, center from center
                property real cropHoffset: 0//0.5

                Item {
                    id: cropmarker
                    visible: root.cropping
                    property real tempHeight: bottomImage.height * Math.max(Math.min(1,imageRect.cropHeight),0)
                    property real tempAspect: imageRect.cropAspect > 10000 ? 10000 : (imageRect.cropAspect < 0.0001 ? 0.0001 : imageRect.cropAspect)
                    width: Math.round(Math.min(tempHeight * tempAspect, bottomImage.width))
                    height: Math.round(Math.min(tempHeight, width / tempAspect))
                    property real maxHoffset: (1-(width /bottomImage.width ))/2
                    property real maxVoffset: (1-(height/bottomImage.height))/2
                    property real oddH: Math.round((bottomImage.width - width)/2)*2 === (bottomImage.width - width) ? 0 : 0.5//it's 0.5 if odd.
                    property real oddV: Math.round((bottomImage.height - height)/2)*2 === (bottomImage.height - height) ? 0 : 0.5// it's 0.5 if odd.
                    property real hoffset: (Math.round(Math.max(Math.min(imageRect.cropHoffset, maxHoffset), -maxHoffset)*bottomImage.width+oddH)-oddH)/bottomImage.width
                    property real voffset: (Math.round(Math.max(Math.min(imageRect.cropVoffset, maxVoffset), -maxVoffset)*bottomImage.height+oddV)-oddV)/bottomImage.height
                    x: bottomImage.x + Math.round(0.5*(bottomImage.width -width)*bottomImage.scale  + hoffset*bottomImage.width*bottomImage.scale)
                    y: bottomImage.y + Math.round(0.5*(bottomImage.height-height)*bottomImage.scale + voffset*bottomImage.height*bottomImage.scale)

                    property bool tooSmall: width*bottomImage.scale/uiScale < parent.cropHandleWidth || height*bottomImage.scale/uiScale < parent.cropHandleWidth
                    transform: Scale {//The scale happens after positioning, about the origin.
                        origin.x: 0
                        origin.y: 0
                        xScale: bottomImage.scale
                        yScale: bottomImage.scale
                    }
                    Rectangle {
                        id: aspectBox
                        anchors.centerIn: parent
                        width: 80
                        height: 30
                        color: "black"
                        border.color: "gray"
                        opacity: 0.8
                        radius: 10
                        Text {
                            id: aspectBoxText
                            anchors.centerIn: parent
                            width: parent.width
                            //text: imageRect.aspectText
                            color: "white"
                            font.pixelSize: 20
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                        }
                        state: "notVisible"
                        states: [
                            State {
                                name: "notVisible"
                                PropertyChanges {
                                    target: aspectBox
                                    opacity: 0
                                }
                            },
                            State {
                                name: "isVisible"
                                PropertyChanges {
                                    target: aspectBox
                                    opacity: 1
                                }
                            }
                        ]
                        transitions: [
                            Transition {
                                from: "isVisible"
                                to: "notVisible"
                                PropertyAnimation {
                                    target: aspectBox
                                    properties: "opacity"
                                    duration: 600
                                }
                            },
                            Transition {
                                from: "notVisible"
                                to: "isVisible"
                                PropertyAnimation {
                                    target: aspectBox
                                    properties: "opacity"
                                    duration: 250
                                }
                            }
                        ]
                        Connections {
                            target: imageRect
                            onAspectTextChanged: {
                                if (imageRect.aspectText == "") {
                                    aspectBox.state = "notVisible"
                                } else {
                                    aspectBoxText.text = imageRect.aspectText
                                    aspectBox.state = "isVisible"
                                    aspectTimer.start()
                                }
                            }
                        }
                        Timer {
                            id: aspectTimer
                            interval: 2000
                            onTriggered: aspectBox.state = "notVisible"
                        }

                        scale: uiScale/bottomImage.scale
                    }
                }

                Rectangle {
                    id: shadeleft
                    color: photoBox.backgroundColor == 2 ? "white" : photoBox.backgroundColor == 1 ? "gray" : "black"
                    //color: "yellow"
                    opacity: 0.5
                    visible: root.cropping
                    x: 0
                    y: 0
                    width: bottomImage.x/bottomImage.scale + (0.5 + cropmarker.hoffset)*bottomImage.width - 0.5*cropmarker.width
                    height: (1 + parent.height)/bottomImage.scale
                    transform: Scale {//The scale happens after positioning, about the origin.
                        origin.x: 0
                        origin.y: 0
                        xScale: bottomImage.scale
                        yScale: bottomImage.scale
                    }
                }
                Rectangle {
                    id: shaderight
                    color: photoBox.backgroundColor == 2 ? "white" : photoBox.backgroundColor == 1 ? "gray" : "black"
                    //color: "yellow"
                    opacity: 0.5
                    visible: root.cropping
                    x: bottomImage.x + ((0.5 + cropmarker.hoffset)*bottomImage.width + 0.5*cropmarker.width)*bottomImage.scale
                    y: 0
                    width: (parent.width - bottomImage.x)/bottomImage.scale - (0.5 + cropmarker.hoffset)*bottomImage.width - 0.5*cropmarker.width + bottomImage.scale
                    height: (1 + parent.height)/bottomImage.scale
                    transform: Scale {//The scale happens after positioning, about the origin.
                        origin.x: 0
                        origin.y: 0
                        xScale: bottomImage.scale
                        yScale: bottomImage.scale
                    }
                }
                Rectangle {
                    id: shadetop
                    color: photoBox.backgroundColor == 2 ? "white" : photoBox.backgroundColor == 1 ? "gray" : "black"
                    //color: "red"
                    opacity: 0.5
                    visible: root.cropping
                    x: 0
                    y: 0
                    width: (1 + parent.width)/bottomImage.scale
                    height: bottomImage.y/bottomImage.scale + (0.5 + cropmarker.voffset)*bottomImage.height - 0.5*cropmarker.height
                    //width: bottomImage.x/bottomImage.scale + (0.5 + cropmarker.hoffset)*bottomImage.width - 0.5*cropmarker.width
                    //height: parent.height/bottomImage.scale
                    transform: Scale {//The scale happens after positioning, about the origin.
                        origin.x: 0
                        origin.y: 0
                        xScale: bottomImage.scale
                        yScale: bottomImage.scale
                    }
                }
                Rectangle {
                    id: shadebottom
                    color: photoBox.backgroundColor == 2 ? "white" : photoBox.backgroundColor == 1 ? "gray" : "black"
                    //color: "red"
                    opacity: 0.5
                    visible: root.cropping
                    x: 0
                    y: bottomImage.y + ((0.5 + cropmarker.voffset)*bottomImage.height + 0.5*cropmarker.height)*bottomImage.scale
                    //x: bottomImage.x + ((0.5 + cropmarker.hoffset)*bottomImage.width + 0.5*cropmarker.width)*bottomImage.scale
                    //y: 0
                    width: (1 + parent.width)/bottomImage.scale
                    height: (parent.height - bottomImage.y)/bottomImage.scale - (0.5 + cropmarker.voffset)*bottomImage.height - 0.5*cropmarker.height + bottomImage.scale
                    //width: (parent.width - bottomImage.x)/bottomImage.scale - (0.5 + cropmarker.hoffset)*bottomImage.width - 0.5*cropmarker.width
                    //height: parent.height/bottomImage.scale
                    transform: Scale {//The scale happens after positioning, about the origin.
                        origin.x: 0
                        origin.y: 0
                        xScale: bottomImage.scale
                        yScale: bottomImage.scale
                    }
                }

                property real cropHandleWidth: 30
                Rectangle {
                    id: cropleft
                    color: 'blue'
                    opacity: 0.5
                    visible: root.cropping && cropResizeLeft.handleVisible
                    width: imageRect.cropHandleWidth*uiScale/bottomImage.scale
                    anchors.top: cropmarker.top
                    anchors.bottom: cropmarker.bottom
                    x: bottomImage.x + Math.round(0.5*(bottomImage.width-2*width)*bottomImage.scale + (cropmarker.hoffset - cropmarker.width/(2*bottomImage.width))*bottomImage.width*bottomImage.scale)
                    transform: Scale {//The scale happens after positioning, about the origin.
                        origin.x: 0
                        origin.y: 0
                        xScale: bottomImage.scale
                        yScale: bottomImage.scale
                    }
                }
                Rectangle {
                    id: cropright
                    color: 'blue'
                    opacity: 0.5
                    visible: root.cropping && cropResizeRight.handleVisible
                    width: imageRect.cropHandleWidth*uiScale/bottomImage.scale
                    anchors.top: cropmarker.top
                    anchors.bottom: cropmarker.bottom
                    x: bottomImage.x + Math.round(0.5*(bottomImage.width)*bottomImage.scale + (cropmarker.hoffset + cropmarker.width/(2*bottomImage.width))*bottomImage.width*bottomImage.scale)
                    transform: Scale {//The scale happens after positioning, about the origin.
                        origin.x: 0
                        origin.y: 0
                        xScale: bottomImage.scale
                        yScale: bottomImage.scale
                    }
                }
                Rectangle {
                    id: croptop
                    color: 'blue'
                    opacity: 0.5
                    visible: root.cropping && cropResizeTop.handleVisible
                    height: imageRect.cropHandleWidth*uiScale/bottomImage.scale
                    anchors.left: cropmarker.left
                    anchors.right: cropmarker.right
                    y: bottomImage.y + Math.round(0.5*(bottomImage.height-2*height)*bottomImage.scale + (cropmarker.voffset - cropmarker.height/(2*bottomImage.height))*bottomImage.height*bottomImage.scale)
                    transform: Scale {//The scale happens after positioning, about the origin.
                        origin.x: 0
                        origin.y: 0
                        xScale: bottomImage.scale
                        yScale: bottomImage.scale
                    }
                }
                Rectangle {
                    id: cropbottom
                    color: 'blue'
                    opacity: 0.5
                    visible: root.cropping && cropResizeBottom.handleVisible
                    height: imageRect.cropHandleWidth*uiScale/bottomImage.scale
                    anchors.left: cropmarker.left
                    anchors.right: cropmarker.right
                    y: bottomImage.y + Math.round(0.5*(bottomImage.height)*bottomImage.scale + (cropmarker.voffset + cropmarker.height/(2*bottomImage.height))*bottomImage.height*bottomImage.scale)
                    transform: Scale {//The scale happens after positioning, about the origin.
                        origin.x: 0
                        origin.y: 0
                        xScale: bottomImage.scale
                        yScale: bottomImage.scale
                    }
                }
                Rectangle {
                    id: croptopleft
                    color: 'purple'
                    opacity: 0.5
                    visible: root.cropping && (cropResizeTopLeft.handleVisible || cropmarker.tooSmall)
                    width:  imageRect.cropHandleWidth*uiScale/bottomImage.scale
                    height: imageRect.cropHandleWidth*uiScale/bottomImage.scale
                    x: bottomImage.x + Math.round(0.5*(bottomImage.width-2*width)*bottomImage.scale + (cropmarker.hoffset - cropmarker.width/(2*bottomImage.width))*bottomImage.width*bottomImage.scale)
                    y: bottomImage.y + Math.round(0.5*(bottomImage.height-2*height)*bottomImage.scale + (cropmarker.voffset - cropmarker.height/(2*bottomImage.height))*bottomImage.height*bottomImage.scale)
                    transform: Scale {//The scale happens after positioning, about the origin.
                        origin.x: 0
                        origin.y: 0
                        xScale: bottomImage.scale
                        yScale: bottomImage.scale
                    }
                }
                Rectangle {
                    id: croptopright
                    color: 'purple'
                    opacity: 0.5
                    visible: root.cropping && (cropResizeTopRight.handleVisible || cropmarker.tooSmall)
                    width:  imageRect.cropHandleWidth*uiScale/bottomImage.scale
                    height: imageRect.cropHandleWidth*uiScale/bottomImage.scale
                    x: bottomImage.x + Math.round(0.5*(bottomImage.width)*bottomImage.scale + (cropmarker.hoffset + cropmarker.width/(2*bottomImage.width))*bottomImage.width*bottomImage.scale)
                    y: bottomImage.y + Math.round(0.5*(bottomImage.height-2*height)*bottomImage.scale + (cropmarker.voffset - cropmarker.height/(2*bottomImage.height))*bottomImage.height*bottomImage.scale)
                    transform: Scale {//The scale happens after positioning, about the origin.
                        origin.x: 0
                        origin.y: 0
                        xScale: bottomImage.scale
                        yScale: bottomImage.scale
                    }
                }
                Rectangle {
                    id: cropbottomleft
                    color: 'purple'
                    opacity: 0.5
                    visible: root.cropping && (cropResizeBottomLeft.handleVisible || cropmarker.tooSmall)
                    width:  imageRect.cropHandleWidth*uiScale/bottomImage.scale
                    height: imageRect.cropHandleWidth*uiScale/bottomImage.scale
                    x: bottomImage.x + Math.round(0.5*(bottomImage.width-2*width)*bottomImage.scale + (cropmarker.hoffset - cropmarker.width/(2*bottomImage.width))*bottomImage.width*bottomImage.scale)
                    y: bottomImage.y + Math.round(0.5*(bottomImage.height)*bottomImage.scale + (cropmarker.voffset + cropmarker.height/(2*bottomImage.height))*bottomImage.height*bottomImage.scale)
                    transform: Scale {//The scale happens after positioning, about the origin.
                        origin.x: 0
                        origin.y: 0
                        xScale: bottomImage.scale
                        yScale: bottomImage.scale
                    }
                }
                Rectangle {
                    id: cropbottomright
                    color: 'purple'
                    opacity: 0.5
                    visible: root.cropping && (cropResizeBottomRight.handleVisible || cropmarker.tooSmall)
                    width:  imageRect.cropHandleWidth*uiScale/bottomImage.scale
                    height: imageRect.cropHandleWidth*uiScale/bottomImage.scale
                    x: bottomImage.x + Math.round(0.5*(bottomImage.width)*bottomImage.scale + (cropmarker.hoffset + cropmarker.width/(2*bottomImage.width))*bottomImage.width*bottomImage.scale)
                    y: bottomImage.y + Math.round(0.5*(bottomImage.height)*bottomImage.scale + (cropmarker.voffset + cropmarker.height/(2*bottomImage.height))*bottomImage.height*bottomImage.scale)
                    transform: Scale {//The scale happens after positioning, about the origin.
                        origin.x: 0
                        origin.y: 0
                        xScale: bottomImage.scale
                        yScale: bottomImage.scale
                    }
                }

                //Readouts of the properties for writing back to database.
                property real readHeight: cropmarker.height / bottomImage.height
                property real readAspect: cropmarker.width / cropmarker.height
                property real readVoffset: cropmarker.voffset
                property real readHoffset: cropmarker.hoffset
                //For showing on the screen
                property real displayWidth:  cropmarker.width
                property real displayHeight: cropmarker.height
                property real displayHoffset: 0.5 * Math.round(2 * cropmarker.hoffset * bottomImage.width)
                property real displayVoffset: 0.5 * Math.round(2 * cropmarker.voffset * bottomImage.height)
                //aspect ratio text
                property string aspectText: ""

                function validateCrop() {
                    imageRect.cropHeight = imageRect.readHeight
                    imageRect.cropAspect = imageRect.readAspect
                    imageRect.cropHoffset = imageRect.readHoffset
                    imageRect.cropVoffset = imageRect.readVoffset
                }

                //Snapping sensitivity in fraction of image height
                property real snapFrac: 0.05

                MouseArea {
                    id: cropDrag
                    acceptedButtons: Qt.LeftButton
                    enabled: false
                    hoverEnabled: true
                    property real cropHeight
                    property real cropAspect
                    property real cropVoffset
                    property real cropHoffset
                    property real tempHeight: bottomImage.height * Math.max(Math.min(1,cropHeight),0)
                    property real tempAspect: cropAspect <= 0 ? 1 : cropAspect
                    width: Math.round(Math.min(tempHeight * tempAspect, bottomImage.width))
                    height: Math.round(Math.min(tempHeight, width / tempAspect))
                    property real maxHoffset: (1-(width /bottomImage.width ))/2
                    property real maxVoffset: (1-(height/bottomImage.height))/2
                    property real hoffset: Math.max(Math.min(cropHoffset, maxHoffset), -maxHoffset)
                    property real voffset: Math.max(Math.min(cropVoffset, maxVoffset), -maxVoffset)
                    x: bottomImage.x + Math.round(0.5*(bottomImage.width -width)*bottomImage.scale  + hoffset*bottomImage.width*bottomImage.scale)
                    y: bottomImage.y + Math.round(0.5*(bottomImage.height-height)*bottomImage.scale + voffset*bottomImage.height*bottomImage.scale)

                    function updatePosition() {
                        cropDrag.cropHeight = imageRect.cropHeight
                        cropDrag.cropAspect = imageRect.cropAspect
                        cropDrag.cropHoffset = imageRect.cropHoffset
                        cropDrag.cropVoffset = imageRect.cropVoffset
                    }

                    transform: Scale {
                        origin.x: 0
                        origin.y: 0
                        xScale: bottomImage.scale
                        yScale: bottomImage.scale
                    }
                    Connections {
                        target: root
                        onCroppingChanged: {
                            if (cropping) {
                                cropDrag.updatePosition()
                                if (root.imageReady) {
                                    cropDrag.enabled = true//case for no quick preview
                                } else {
                                    cropDrag.enabled = false//for quick preview
                                }
                                cropDrag.visible = true
                            }
                            else {
                                cropDrag.enabled = false
                                cropDrag.visible = false
                            }
                        }
                        onImageReadyChanged: {
                            if (cropping) {
                                cropDrag.enabled = true//only needed for quick preview
                            }
                        }
                    }

                    property real oldX
                    property real oldY
                    property real unclippedHoffset
                    property real unclippedVoffset
                    property real ctrlPressed
                    onPressed: {
                        imageRect.validateCrop()
                        if (mouse.modifiers & Qt.ControlModifier) {//ctrl pressed at the beginning initiates drag
                            preventStealing = true
                            ctrlPressed = true
                        } else {
                            preventStealing = false
                            ctrlPressed = false
                        }
                        oldX = mouse.x
                        oldY = mouse.y
                        unclippedHoffset = cropmarker.hoffset
                        unclippedVoffset = cropmarker.voffset
                    }
                    onPositionChanged: {
                        if (ctrlPressed) {
                            var deltaX = mouse.x - oldX
                            var deltaY = mouse.y - oldY
                            oldX = mouse.x
                            oldY = mouse.y
                            unclippedHoffset = unclippedHoffset + deltaX/bottomImage.width
                            unclippedVoffset = unclippedVoffset + deltaY/bottomImage.height
                            if (mouse.modifiers & Qt.ShiftModifier) {//shift modifier pressed; snap to center
                                if (Math.abs(unclippedHoffset) < imageRect.snapFrac) {
                                    imageRect.cropHoffset = 0
                                } else {
                                    imageRect.cropHoffset = unclippedHoffset
                                }
                                if (Math.abs(unclippedVoffset) < imageRect.snapFrac) {
                                    imageRect.cropVoffset = 0
                                } else {
                                    imageRect.cropVoffset = unclippedVoffset
                                }
                            } else {
                                imageRect.cropHoffset = unclippedHoffset
                                imageRect.cropVoffset = unclippedVoffset
                            }
                        }
                    }
                    onReleased: {
                        preventStealing = false
                        ctrlPressed = false
                        imageRect.validateCrop()
                        cropDrag.updatePosition()
                    }
                    onDoubleClicked: {//we want it to behave like the background when double-clicked if ctrl isn't held
                        if (!(mouse.modifiers & Qt.ControlModifier)) {
                            //the following block is taken straight from doubleClickCapture
                            if (bottomImage.scale < flicky.fitScale || bottomImage.scale == 1) {
                                bottomImage.scale = flicky.fitScale
                                flicky.contentX = Math.floor(cropMargin*uiScale*cropping)
                                flicky.contentY = Math.floor(cropMargin*uiScale*cropping)
                                flicky.fit = true
                            } else {//this is modified from doubleClickCapture
                                var zoomFactor = 1/bottomImage.scale

                                var oldContentX = flicky.contentX
                                var oldContentY = flicky.contentY

                                //oldMouseX is the screen-space distance from the left of the image.
                                //This is what it was in doubleClickCapture
                                //var oldMouseX = mouse.x - Math.max(0, 0.5*(flicky.width  - bottomImage.width*bottomImage.scale))  - 2*Math.floor(cropMargin*uiScale*cropping)

                                //But here the formula for this is:
                                var oldMouseX = (mouse.x + (0.5 + imageRect.cropHoffset)*bottomImage.width - 0.5*imageRect.cropHeight*imageRect.cropAspect*bottomImage.height)*bottomImage.scale
                                var oldMouseY = (mouse.y + (0.5 + imageRect.cropVoffset - 0.5*imageRect.cropHeight)*bottomImage.height)*bottomImage.scale

                                //Now we need to reverse-engineer the doubleClickCapture mouse.x and such, because when we change the scale here our mouse.x goes haywire.
                                var oldDCCMouseX = oldMouseX + Math.max(0, 0.5*(flicky.width  - bottomImage.width*bottomImage.scale))  + 2*Math.floor(cropMargin*uiScale*cropping)
                                var oldDCCMouseY = oldMouseY + Math.max(0, 0.5*(flicky.height - bottomImage.height*bottomImage.scale)) + 2*Math.floor(cropMargin*uiScale*cropping)

                                bottomImage.scale = 1

                                //Now we need to compute the new doubleClickCapture mouse.x from the old one
                                var newDCCMouseX = oldDCCMouseX - Math.max(0, 0.5*(flicky.width  - bottomImage.width))  - 2*Math.floor(cropMargin*uiScale*cropping)
                                var newDCCMouseY = oldDCCMouseY - Math.max(0, 0.5*(flicky.height - bottomImage.height)) - 2*Math.floor(cropMargin*uiScale*cropping)

                                //The pattern is  oldcontentX + oldMouseX*zoomfactor - recalculated oldMouseX, but we use the doubleClickCapture version
                                flicky.contentX = oldContentX + oldMouseX*zoomFactor - newDCCMouseX
                                flicky.contentY = oldContentY + oldMouseY*zoomFactor - newDCCMouseY

                                flicky.returnToBounds()
                                if (bottomImage.scale == flicky.fitScale) {flicky.fit = true}
                                else {flicky.fit = false}
                            }
                        }
                    }
                }
                MouseArea {
                    id: cropResizeLeft
                    acceptedButtons: Qt.LeftButton
                    hoverEnabled: true
                    enabled: cropDrag.enabled
                    visible: cropDrag.visible
                    width: imageRect.cropHandleWidth*uiScale/bottomImage.scale
                    anchors.top: cropDrag.top
                    anchors.bottom: cropDrag.bottom
                    x: bottomImage.x + Math.round(0.5*(bottomImage.width-2*width)*bottomImage.scale + (cropDrag.hoffset - cropDrag.width/(2*bottomImage.width))*bottomImage.width*bottomImage.scale)
                    transform: Scale {
                        origin.x: 0
                        origin.y: 0
                        xScale: bottomImage.scale
                        yScale: bottomImage.scale
                    }

                    property real oldX
                    property real oldWidth
                    property real unclippedWidth
                    property real oldOffset
                    property real unclippedHoffset

                    property bool handleVisible: false
                    onEntered: {
                        handleVisible = true
                    }
                    onExited: {
                        if (!pressed) {
                            handleVisible = false
                        }
                    }
                    onPressed: {
                        imageRect.validateCrop()
                        preventStealing = true
                        oldX = mouse.x
                        oldWidth = cropmarker.width
                        unclippedWidth = cropmarker.width
                        oldOffset = cropmarker.hoffset
                        unclippedHoffset = cropmarker.hoffset
                    }
                    onPositionChanged: {
                        if (pressed) {
                            var deltaX = mouse.x - oldX
                            oldX = mouse.x
                            if (!(mouse.modifiers & Qt.ControlModifier)) {//no modifiers; resize as usual
                                unclippedWidth = unclippedWidth - deltaX//The width of the image after this drag. It may be zero or negative, or impossibly big.
                                //So obviously we want to clip it at 0 before using it, but we should try to keep track of this value.
                                //We also need to keep it from getting too wide.
                                // The full image's width is bottomImage.width, divided by two to start at the middle.
                                // Add the offset (in pixels) to get to the middle of the image.
                                // Add half of the width of the crop itself.
                                //And then we round.
                                var clippedWidth = Math.round(Math.min(Math.max(1,unclippedWidth),bottomImage.width*(0.5+oldOffset)+0.5*oldWidth))
                                imageRect.cropAspect = clippedWidth/(bottomImage.height*imageRect.cropHeight)
                                //Now we want to remember where the right edge of the image was, and preserve that.
                                imageRect.cropHoffset = oldOffset + 0.5*(oldWidth-clippedWidth)/bottomImage.width
                                unclippedHoffset = imageRect.cropHoffset
                            } else {//ctrl modifier pressed; drag in one direction
                                unclippedHoffset = unclippedHoffset + deltaX/bottomImage.width
                                if (mouse.modifiers & Qt.ShiftModifier && Math.abs(unclippedHoffset) < imageRect.snapFrac) {//shift modifier pressed; snap to middle
                                    imageRect.cropHoffset = 0
                                } else {
                                    imageRect.cropHoffset = unclippedHoffset
                                }
                                oldWidth = cropmarker.width
                                unclippedWidth = cropmarker.width
                                oldOffset = imageRect.cropHoffset
                            }
                        }
                    }
                    onReleased: {
                        if (!containsMouse) {
                            handleVisible = false
                        }
                        preventStealing = false
                        imageRect.validateCrop()
                        cropDrag.updatePosition()
                    }
                }
                MouseArea {
                    id: cropResizeRight
                    acceptedButtons: Qt.LeftButton
                    hoverEnabled: true
                    enabled: cropDrag.enabled
                    visible: cropDrag.visible
                    width: imageRect.cropHandleWidth*uiScale/bottomImage.scale
                    anchors.top: cropDrag.top
                    anchors.bottom: cropDrag.bottom
                    x: bottomImage.x + Math.round(0.5*(bottomImage.width)*bottomImage.scale + (cropDrag.hoffset + cropDrag.width/(2*bottomImage.width))*bottomImage.width*bottomImage.scale)
                    transform: Scale {
                        origin.x: 0
                        origin.y: 0
                        xScale: bottomImage.scale
                        yScale: bottomImage.scale
                    }

                    property real oldX
                    property real oldWidth
                    property real unclippedWidth
                    property real oldOffset
                    property real unclippedHoffset

                    property bool handleVisible: false
                    onEntered: {
                        handleVisible = true
                    }
                    onExited: {
                        if (!pressed) {
                            handleVisible = false
                        }
                    }
                    onPressed: {
                        imageRect.validateCrop()
                        preventStealing = true
                        oldX = mouse.x
                        oldWidth = cropmarker.width
                        unclippedWidth = cropmarker.width
                        oldOffset = cropmarker.hoffset
                        unclippedHoffset = cropmarker.hoffset
                    }
                    onPositionChanged: {
                        if (pressed) {
                            var deltaX = mouse.x - oldX
                            oldX = mouse.x
                            if (!(mouse.modifiers & Qt.ControlModifier)) {//no modifiers; resize as usual
                                unclippedWidth = unclippedWidth + deltaX
                                var clippedWidth = Math.round(Math.min(Math.max(1,unclippedWidth),bottomImage.width*(0.5-oldOffset)+0.5*oldWidth))
                                imageRect.cropAspect = clippedWidth/(bottomImage.height*imageRect.cropHeight)
                                //Now we want to remember where the left edge of the image was, and preserve that.
                                imageRect.cropHoffset = oldOffset - 0.5*(oldWidth-clippedWidth)/bottomImage.width
                                unclippedHoffset = imageRect.cropHoffset
                            } else {//ctrl modifier pressed; drag in one direction
                                unclippedHoffset = unclippedHoffset + deltaX/bottomImage.width
                                if (mouse.modifiers & Qt.ShiftModifier && Math.abs(unclippedHoffset) < imageRect.snapFrac) {//shift modifier pressed; snap to middle
                                    imageRect.cropHoffset = 0
                                } else {
                                    imageRect.cropHoffset = unclippedHoffset
                                }
                                oldWidth = cropmarker.width
                                unclippedWidth = cropmarker.width
                                oldOffset = imageRect.cropHoffset
                            }
                        }
                    }
                    onReleased: {
                        if (!containsMouse) {
                            handleVisible = false
                        }
                        preventStealing = false
                        imageRect.validateCrop()
                        cropDrag.updatePosition()
                    }
                }
                MouseArea {
                    id: cropResizeTop
                    acceptedButtons: Qt.LeftButton
                    hoverEnabled: true
                    enabled: cropDrag.enabled
                    visible: cropDrag.visible
                    height: imageRect.cropHandleWidth*uiScale/bottomImage.scale
                    anchors.left: cropDrag.left
                    anchors.right: cropDrag.right
                    y: bottomImage.y + Math.round(0.5*(bottomImage.height-2*height)*bottomImage.scale + (cropDrag.voffset - cropDrag.height/(2*bottomImage.height))*bottomImage.height*bottomImage.scale)
                    transform: Scale {
                        origin.x: 0
                        origin.y: 0
                        xScale: bottomImage.scale
                        yScale: bottomImage.scale
                    }

                    property real oldY
                    property real oldHeight
                    property real unclippedHeight
                    property real oldOffset
                    property real unclippedVoffset

                    property bool handleVisible: false
                    onEntered: {
                        handleVisible = true
                    }
                    onExited: {
                        if (!pressed) {
                            handleVisible = false
                        }
                    }
                    onPressed: {
                        imageRect.validateCrop()
                        preventStealing = true
                        oldY = mouse.y
                        oldHeight = cropmarker.height
                        unclippedHeight = cropmarker.height
                        oldOffset = cropmarker.voffset
                        unclippedVoffset = cropmarker.voffset
                    }
                    onPositionChanged: {
                        if (pressed) {
                            var deltaY = mouse.y - oldY
                            oldY = mouse.y
                            if (!(mouse.modifiers & Qt.ControlModifier)) {//no modifiers; resize as usual
                                unclippedHeight = unclippedHeight - deltaY
                                var clippedHeight = Math.round(Math.min(Math.max(1, unclippedHeight), bottomImage.height*(0.5+oldOffset)+0.5*oldHeight))
                                imageRect.cropAspect = cropDrag.width/clippedHeight
                                imageRect.cropHeight = clippedHeight/bottomImage.height
                                //Remember where the bottom edge is.
                                imageRect.cropVoffset = oldOffset + 0.5*(oldHeight-clippedHeight)/bottomImage.height
                                unclippedVoffset = imageRect.cropVoffset
                            } else {//ctrl modifier pressed; drag in one direction
                                unclippedVoffset = unclippedVoffset + deltaY/bottomImage.height
                                if (mouse.modifiers & Qt.ShiftModifier && Math.abs(unclippedVoffset) < imageRect.snapFrac) {//shift modifier pressed; snap to middle
                                    imageRect.cropVoffset = 0
                                } else {
                                    imageRect.cropVoffset = unclippedVoffset
                                }
                                oldHeight = cropmarker.height
                                unclippedHeight = cropmarker.height
                                oldOffset = imageRect.cropVoffset
                            }
                        }
                    }
                    onReleased: {
                        if (!containsMouse) {
                            handleVisible = false
                        }
                        preventStealing = false
                        imageRect.validateCrop()
                        cropDrag.updatePosition()
                    }
                }
                MouseArea {
                    id: cropResizeBottom
                    acceptedButtons: Qt.LeftButton
                    hoverEnabled: true
                    enabled: cropDrag.enabled
                    visible: cropDrag.visible
                    height: imageRect.cropHandleWidth*uiScale/bottomImage.scale
                    anchors.left: cropDrag.left
                    anchors.right: cropDrag.right
                    y: bottomImage.y + Math.round(0.5*(bottomImage.height)*bottomImage.scale + (cropDrag.voffset + cropDrag.height/(2*bottomImage.height))*bottomImage.height*bottomImage.scale)
                    transform: Scale {
                        origin.x: 0
                        origin.y: 0
                        xScale: bottomImage.scale
                        yScale: bottomImage.scale
                    }

                    property real oldY
                    property real oldHeight
                    property real unclippedHeight
                    property real oldOffset
                    property real unclippedVoffset

                    property bool handleVisible: false
                    onEntered: {
                        handleVisible = true
                    }
                    onExited: {
                        if (!pressed) {
                            handleVisible = false
                        }
                    }
                    onPressed: {
                        imageRect.validateCrop()
                        preventStealing = true
                        oldY = mouse.y
                        oldHeight = cropmarker.height
                        unclippedHeight = cropmarker.height
                        oldOffset = cropmarker.voffset
                        unclippedVoffset = cropmarker.voffset
                    }
                    onPositionChanged: {
                        if (pressed) {
                            var deltaY = mouse.y - oldY
                            oldY = mouse.y
                            if (!(mouse.modifiers & Qt.ControlModifier)) {//no modifiers; resize as usual
                                unclippedHeight = unclippedHeight + deltaY
                                var clippedHeight = Math.round(Math.min(Math.max(1, unclippedHeight), bottomImage.height*(0.5-oldOffset)+0.5*oldHeight))
                                imageRect.cropAspect = cropDrag.width/clippedHeight
                                imageRect.cropHeight = clippedHeight/bottomImage.height
                                //Remember where the bottom edge is.
                                imageRect.cropVoffset = oldOffset - 0.5*(oldHeight-clippedHeight)/bottomImage.height
                                unclippedVoffset = imageRect.cropVoffset
                            } else {//ctrl modifier pressed; drag in one direction
                                unclippedVoffset = unclippedVoffset + deltaY/bottomImage.height
                                if (mouse.modifiers & Qt.ShiftModifier && Math.abs(unclippedVoffset) < imageRect.snapFrac) {//shift modifier pressed; snap to middle
                                    imageRect.cropVoffset = 0
                                } else {
                                    imageRect.cropVoffset = unclippedVoffset
                                }
                                oldHeight = cropmarker.height
                                unclippedHeight = cropmarker.height
                                oldOffset = imageRect.cropVoffset
                            }
                        }
                    }
                    onReleased: {
                        if (!containsMouse) {
                            handleVisible = false
                        }
                        preventStealing = false
                        imageRect.validateCrop()
                        cropDrag.updatePosition()
                    }
                }
                MouseArea {
                    id: cropResizeTopLeft
                    acceptedButtons: Qt.LeftButton
                    hoverEnabled: true
                    enabled: cropDrag.enabled
                    visible: cropDrag.visible
                    width: imageRect.cropHandleWidth*uiScale/bottomImage.scale
                    height: imageRect.cropHandleWidth*uiScale/bottomImage.scale
                    x: bottomImage.x + Math.round(0.5*(bottomImage.width-2*width)*bottomImage.scale + (cropDrag.hoffset - cropDrag.width/(2*bottomImage.width))*bottomImage.width*bottomImage.scale)
                    y: bottomImage.y + Math.round(0.5*(bottomImage.height-2*height)*bottomImage.scale + (cropDrag.voffset - cropDrag.height/(2*bottomImage.height))*bottomImage.height*bottomImage.scale)
                    transform: Scale {
                        origin.x: 0
                        origin.y: 0
                        xScale: bottomImage.scale
                        yScale: bottomImage.scale
                    }

                    property real oldX
                    property real oldY
                    property real oldWidth
                    property real oldHeight
                    property real unclippedWidth
                    property real unclippedHeight
                    property real clippedWidth
                    property real clippedHeight
                    property real oldHoffset
                    property real oldVoffset
                    property real oldAspect
                    property real lockedAspect

                    property bool handleVisible: false
                    onEntered: {
                        handleVisible = true
                    }
                    onExited: {
                        if (!pressed) {
                            handleVisible = false
                        }
                    }
                    onPressed: {
                        imageRect.validateCrop()
                        preventStealing = true
                        oldX = mouse.x
                        oldY = mouse.y
                        oldWidth = cropmarker.width
                        oldHeight = cropmarker.height
                        unclippedWidth = cropmarker.width
                        unclippedHeight = cropmarker.height
                        oldHoffset = cropmarker.hoffset
                        oldVoffset = cropmarker.voffset
                        oldAspect = cropmarker.width/cropmarker.height
                        lockedAspect = oldAspect
                    }
                    onPositionChanged: {
                        if (pressed) {
                            var deltaX = mouse.x - oldX
                            var deltaY = mouse.y - oldY
                            oldX = mouse.x
                            oldY = mouse.y
                            unclippedWidth = unclippedWidth - deltaX
                            unclippedHeight = unclippedHeight - deltaY
                            var clippedWidth = Math.round(Math.min(Math.max(1,unclippedWidth),bottomImage.width*(0.5+oldHoffset)+0.5*oldWidth))
                            var clippedHeight = Math.round(Math.min(Math.max(1, unclippedHeight), bottomImage.height*(0.5+oldVoffset)+0.5*oldHeight))
                            var newAspect = clippedWidth/clippedHeight
                            if (mouse.modifiers & Qt.ShiftModifier && !(mouse.modifiers & Qt.ControlModifier)) {
                                //set aspect to a snapped one
                                if      (newAspect < 0.3478) {lockedAspect = 1/3;    imageRect.aspectText = "1:3"}
                                else if (newAspect < 0.3936) {lockedAspect = 1/2.76; imageRect.aspectText = "1:2.76"}
                                else if (newAspect < 0.4622) {lockedAspect = 1/2.35; imageRect.aspectText = "1:2.35"}
                                else if (newAspect < 0.5308) {lockedAspect = 1/2;    imageRect.aspectText = "1:2"}
                                else if (newAspect < 0.5899) {lockedAspect = 9/16;   imageRect.aspectText = "9:16"}
                                else if (newAspect < 0.6420) {lockedAspect = 0.61803;imageRect.aspectText = "1:φ"}
                                else if (newAspect < 0.7075) {lockedAspect = 2/3;    imageRect.aspectText = "2:3"}
                                else if (newAspect < 0.7747) {lockedAspect = 3/4;    imageRect.aspectText = "3:4"}
                                else if (newAspect < 0.8950) {lockedAspect = 4/5;    imageRect.aspectText = "4:5"}
                                else if (newAspect < 1.1173) {lockedAspect = 1;      imageRect.aspectText = "1:1"}
                                else if (newAspect < 1.2908) {lockedAspect = 5/4;    imageRect.aspectText = "5:4"}
                                else if (newAspect < 1.4134) {lockedAspect = 4/3;    imageRect.aspectText = "4:3"}
                                else if (newAspect < 1.5574) {lockedAspect = 3/2;    imageRect.aspectText = "3:2"}
                                else if (newAspect < 1.6951) {lockedAspect = 1.61803;imageRect.aspectText = "φ:1"}
                                else if (newAspect < 1.8837) {lockedAspect = 16/9;   imageRect.aspectText = "16:9"}
                                else if (newAspect < 2.1634) {lockedAspect = 2;      imageRect.aspectText = "2:1"}
                                else if (newAspect < 2.5407) {lockedAspect = 2.35;   imageRect.aspectText = "2.35:1"}
                                else if (newAspect < 2.8755) {lockedAspect = 2.76;   imageRect.aspectText = "2.76:1"}
                                else                         {lockedAspect = 3;      imageRect.aspectText = "3:1"}
                            }

                            if (mouse.modifiers & Qt.ControlModifier || mouse.modifiers & Qt.ShiftModifier) {
                                //choose whether to use height or width based on aspect, then clip them
                                if (lockedAspect < newAspect) {//narrower, we adjust the height
                                    //Set both width and height explicitly so that the offsets can be corrected later.
                                    clippedHeight = Math.round(Math.min(Math.max(1, clippedWidth/lockedAspect),bottomImage.height*(0.5+oldVoffset)+0.5*oldHeight))
                                    clippedWidth = clippedHeight*lockedAspect
                                    imageRect.cropHeight = clippedHeight/bottomImage.height
                                } else {
                                    clippedWidth = Math.round(Math.min(Math.max(1,clippedHeight*lockedAspect),bottomImage.width*(0.5+oldHoffset)+0.5*oldWidth))
                                    clippedHeight = clippedWidth/lockedAspect
                                    imageRect.cropHeight = clippedHeight/bottomImage.height
                                }
                                //then set aspect back to the original in case of snapping.
                                imageRect.cropAspect = lockedAspect
                            } else {
                                lockedAspect = newAspect
                                imageRect.cropHeight = clippedHeight/bottomImage.height
                                imageRect.cropAspect = newAspect
                            }
                            //Now we want to remember where the right edge of the image was, and preserve that.
                            imageRect.cropHoffset = oldHoffset + 0.5*(oldWidth-clippedWidth)/bottomImage.width
                            imageRect.cropVoffset = oldVoffset + 0.5*(oldHeight-clippedHeight)/bottomImage.height
                        }
                    }
                    onReleased: {
                        if (!containsMouse) {
                            handleVisible = false
                        }
                        imageRect.aspectText = ""
                        preventStealing = false
                        imageRect.validateCrop()
                        cropDrag.updatePosition()
                    }
                }
                MouseArea {
                    id: cropResizeTopRight
                    acceptedButtons: Qt.LeftButton
                    hoverEnabled: true
                    enabled: cropDrag.enabled
                    visible: cropDrag.visible
                    width: imageRect.cropHandleWidth*uiScale/bottomImage.scale
                    height: imageRect.cropHandleWidth*uiScale/bottomImage.scale
                    x: bottomImage.x + Math.round(0.5*(bottomImage.width)*bottomImage.scale + (cropDrag.hoffset + cropDrag.width/(2*bottomImage.width))*bottomImage.width*bottomImage.scale)
                    y: bottomImage.y + Math.round(0.5*(bottomImage.height-2*height)*bottomImage.scale + (cropDrag.voffset - cropDrag.height/(2*bottomImage.height))*bottomImage.height*bottomImage.scale)
                    transform: Scale {
                        origin.x: 0
                        origin.y: 0
                        xScale: bottomImage.scale
                        yScale: bottomImage.scale
                    }

                    property real oldX
                    property real oldY
                    property real oldWidth
                    property real oldHeight
                    property real unclippedWidth
                    property real unclippedHeight
                    property real clippedWidth
                    property real clippedHeight
                    property real oldHoffset
                    property real oldVoffset
                    property real oldAspect
                    property real lockedAspect

                    property bool handleVisible: false
                    onEntered: {
                        handleVisible = true
                    }
                    onExited: {
                        if (!pressed) {
                            handleVisible = false
                        }
                    }
                    onPressed: {
                        imageRect.validateCrop()
                        preventStealing = true
                        oldX = mouse.x
                        oldY = mouse.y
                        oldWidth = cropmarker.width
                        oldHeight = cropmarker.height
                        unclippedWidth = cropmarker.width
                        unclippedHeight = cropmarker.height
                        oldHoffset = cropmarker.hoffset
                        oldVoffset = cropmarker.voffset
                        oldAspect = cropmarker.width/cropmarker.height
                        lockedAspect = oldAspect
                    }
                    onPositionChanged: {
                        if (pressed) {
                            var deltaX = mouse.x - oldX
                            var deltaY = mouse.y - oldY
                            oldX = mouse.x
                            oldY = mouse.y
                            unclippedWidth = unclippedWidth + deltaX
                            unclippedHeight = unclippedHeight - deltaY
                            var clippedWidth = Math.round(Math.min(Math.max(1,unclippedWidth),bottomImage.width*(0.5-oldHoffset)+0.5*oldWidth))
                            var clippedHeight = Math.round(Math.min(Math.max(1, unclippedHeight), bottomImage.height*(0.5+oldVoffset)+0.5*oldHeight))
                            var newAspect = clippedWidth/clippedHeight
                            if (mouse.modifiers & Qt.ShiftModifier && !(mouse.modifiers & Qt.ControlModifier)) {
                                //set aspect to a snapped one
                                if      (newAspect < 0.3478) {lockedAspect = 1/3;    imageRect.aspectText = "1:3"}
                                else if (newAspect < 0.3936) {lockedAspect = 1/2.76; imageRect.aspectText = "1:2.76"}
                                else if (newAspect < 0.4622) {lockedAspect = 1/2.35; imageRect.aspectText = "1:2.35"}
                                else if (newAspect < 0.5308) {lockedAspect = 1/2;    imageRect.aspectText = "1:2"}
                                else if (newAspect < 0.5899) {lockedAspect = 9/16;   imageRect.aspectText = "9:16"}
                                else if (newAspect < 0.6420) {lockedAspect = 0.61803;imageRect.aspectText = "1:φ"}
                                else if (newAspect < 0.7075) {lockedAspect = 2/3;    imageRect.aspectText = "2:3"}
                                else if (newAspect < 0.7747) {lockedAspect = 3/4;    imageRect.aspectText = "3:4"}
                                else if (newAspect < 0.8950) {lockedAspect = 4/5;    imageRect.aspectText = "4:5"}
                                else if (newAspect < 1.1173) {lockedAspect = 1;      imageRect.aspectText = "1:1"}
                                else if (newAspect < 1.2908) {lockedAspect = 5/4;    imageRect.aspectText = "5:4"}
                                else if (newAspect < 1.4134) {lockedAspect = 4/3;    imageRect.aspectText = "4:3"}
                                else if (newAspect < 1.5574) {lockedAspect = 3/2;    imageRect.aspectText = "3:2"}
                                else if (newAspect < 1.6951) {lockedAspect = 1.61803;imageRect.aspectText = "φ:1"}
                                else if (newAspect < 1.8837) {lockedAspect = 16/9;   imageRect.aspectText = "16:9"}
                                else if (newAspect < 2.1634) {lockedAspect = 2;      imageRect.aspectText = "2:1"}
                                else if (newAspect < 2.5407) {lockedAspect = 2.35;   imageRect.aspectText = "2.35:1"}
                                else if (newAspect < 2.8755) {lockedAspect = 2.76;   imageRect.aspectText = "2.76:1"}
                                else                         {lockedAspect = 3;      imageRect.aspectText = "3:1"}
                            }

                            if (mouse.modifiers & Qt.ControlModifier || mouse.modifiers & Qt.ShiftModifier) {
                                //choose whether to use height or width based on aspect, then clip them
                                if (lockedAspect < newAspect) {//narrower, we adjust the height
                                    //Set both width and height explicitly so that the offsets can be corrected later.
                                    clippedHeight = Math.round(Math.min(Math.max(1, clippedWidth/lockedAspect),bottomImage.height*(0.5+oldVoffset)+0.5*oldHeight))
                                    clippedWidth = clippedHeight*lockedAspect
                                    imageRect.cropHeight = clippedHeight/bottomImage.height
                                } else {
                                    clippedWidth = Math.round(Math.min(Math.max(1,clippedHeight*lockedAspect),bottomImage.width*(0.5-oldHoffset)+0.5*oldWidth))
                                    clippedHeight = clippedWidth/lockedAspect
                                    imageRect.cropHeight = clippedHeight/bottomImage.height
                                }
                                //then set aspect back to the original in case of snapping.
                                imageRect.cropAspect = lockedAspect
                            } else {
                                lockedAspect = newAspect
                                imageRect.cropHeight = clippedHeight/bottomImage.height
                                imageRect.cropAspect = newAspect
                            }
                            //Now we want to remember where the right edge of the image was, and preserve that.
                            imageRect.cropHoffset = oldHoffset - 0.5*(oldWidth-clippedWidth)/bottomImage.width
                            imageRect.cropVoffset = oldVoffset + 0.5*(oldHeight-clippedHeight)/bottomImage.height
                        }
                    }
                    onReleased: {
                        if (!containsMouse) {
                            handleVisible = false
                        }
                        imageRect.aspectText = ""
                        preventStealing = false
                        imageRect.validateCrop()
                        cropDrag.updatePosition()
                    }
                }
                MouseArea {
                    id: cropResizeBottomLeft
                    acceptedButtons: Qt.LeftButton
                    hoverEnabled: true
                    enabled: cropDrag.enabled
                    visible: cropDrag.visible
                    width: imageRect.cropHandleWidth*uiScale/bottomImage.scale
                    height: imageRect.cropHandleWidth*uiScale/bottomImage.scale
                    x: bottomImage.x + Math.round(0.5*(bottomImage.width-2*width)*bottomImage.scale + (cropDrag.hoffset - cropDrag.width/(2*bottomImage.width))*bottomImage.width*bottomImage.scale)
                    y: bottomImage.y + Math.round(0.5*(bottomImage.height)*bottomImage.scale + (cropDrag.voffset + cropDrag.height/(2*bottomImage.height))*bottomImage.height*bottomImage.scale)
                    transform: Scale {
                        origin.x: 0
                        origin.y: 0
                        xScale: bottomImage.scale
                        yScale: bottomImage.scale
                    }

                    property real oldX
                    property real oldY
                    property real oldWidth
                    property real oldHeight
                    property real unclippedWidth
                    property real unclippedHeight
                    property real clippedWidth
                    property real clippedHeight
                    property real oldHoffset
                    property real oldVoffset
                    property real oldAspect
                    property real lockedAspect

                    property bool handleVisible: false
                    onEntered: {
                        handleVisible = true
                    }
                    onExited: {
                        if (!pressed) {
                            handleVisible = false
                        }
                    }
                    onPressed: {
                        imageRect.validateCrop()
                        preventStealing = true
                        oldX = mouse.x
                        oldY = mouse.y
                        oldWidth = cropmarker.width
                        oldHeight = cropmarker.height
                        unclippedWidth = cropmarker.width
                        unclippedHeight = cropmarker.height
                        oldHoffset = cropmarker.hoffset
                        oldVoffset = cropmarker.voffset
                        oldAspect = cropmarker.width/cropmarker.height
                        lockedAspect = oldAspect
                    }
                    onPositionChanged: {
                        if (pressed) {
                            var deltaX = mouse.x - oldX
                            var deltaY = mouse.y - oldY
                            oldX = mouse.x
                            oldY = mouse.y
                            unclippedWidth = unclippedWidth - deltaX
                            unclippedHeight = unclippedHeight + deltaY
                            var clippedWidth = Math.round(Math.min(Math.max(1,unclippedWidth),bottomImage.width*(0.5+oldHoffset)+0.5*oldWidth))
                            var clippedHeight = Math.round(Math.min(Math.max(1, unclippedHeight), bottomImage.height*(0.5-oldVoffset)+0.5*oldHeight))
                            var newAspect = clippedWidth/clippedHeight
                            if (mouse.modifiers & Qt.ShiftModifier && !(mouse.modifiers & Qt.ControlModifier)) {
                                //set aspect to a snapped one
                                if      (newAspect < 0.3478) {lockedAspect = 1/3;    imageRect.aspectText = "1:3"}
                                else if (newAspect < 0.3936) {lockedAspect = 1/2.76; imageRect.aspectText = "1:2.76"}
                                else if (newAspect < 0.4622) {lockedAspect = 1/2.35; imageRect.aspectText = "1:2.35"}
                                else if (newAspect < 0.5308) {lockedAspect = 1/2;    imageRect.aspectText = "1:2"}
                                else if (newAspect < 0.5899) {lockedAspect = 9/16;   imageRect.aspectText = "9:16"}
                                else if (newAspect < 0.6420) {lockedAspect = 0.61803;imageRect.aspectText = "1:φ"}
                                else if (newAspect < 0.7075) {lockedAspect = 2/3;    imageRect.aspectText = "2:3"}
                                else if (newAspect < 0.7747) {lockedAspect = 3/4;    imageRect.aspectText = "3:4"}
                                else if (newAspect < 0.8950) {lockedAspect = 4/5;    imageRect.aspectText = "4:5"}
                                else if (newAspect < 1.1173) {lockedAspect = 1;      imageRect.aspectText = "1:1"}
                                else if (newAspect < 1.2908) {lockedAspect = 5/4;    imageRect.aspectText = "5:4"}
                                else if (newAspect < 1.4134) {lockedAspect = 4/3;    imageRect.aspectText = "4:3"}
                                else if (newAspect < 1.5574) {lockedAspect = 3/2;    imageRect.aspectText = "3:2"}
                                else if (newAspect < 1.6951) {lockedAspect = 1.61803;imageRect.aspectText = "φ:1"}
                                else if (newAspect < 1.8837) {lockedAspect = 16/9;   imageRect.aspectText = "16:9"}
                                else if (newAspect < 2.1634) {lockedAspect = 2;      imageRect.aspectText = "2:1"}
                                else if (newAspect < 2.5407) {lockedAspect = 2.35;   imageRect.aspectText = "2.35:1"}
                                else if (newAspect < 2.8755) {lockedAspect = 2.76;   imageRect.aspectText = "2.76:1"}
                                else                         {lockedAspect = 3;      imageRect.aspectText = "3:1"}
                            }

                            if (mouse.modifiers & Qt.ControlModifier || mouse.modifiers & Qt.ShiftModifier) {
                                //choose whether to use height or width based on aspect, then clip them
                                if (lockedAspect < newAspect) {//narrower, we adjust the height
                                    //Set both width and height explicitly so that the offsets can be corrected later.
                                    clippedHeight = Math.round(Math.min(Math.max(1, clippedWidth/lockedAspect),bottomImage.height*(0.5-oldVoffset)+0.5*oldHeight))
                                    clippedWidth = clippedHeight*lockedAspect
                                    imageRect.cropHeight = clippedHeight/bottomImage.height
                                } else {
                                    clippedWidth = Math.round(Math.min(Math.max(1,clippedHeight*lockedAspect),bottomImage.width*(0.5+oldHoffset)+0.5*oldWidth))
                                    clippedHeight = clippedWidth/lockedAspect
                                    imageRect.cropHeight = clippedHeight/bottomImage.height
                                }
                                //then set aspect back to the original in case of snapping.
                                imageRect.cropAspect = lockedAspect
                            } else {
                                lockedAspect = newAspect
                                imageRect.cropHeight = clippedHeight/bottomImage.height
                                imageRect.cropAspect = newAspect
                            }
                            //Now we want to remember where the right edge of the image was, and preserve that.
                            imageRect.cropHoffset = oldHoffset + 0.5*(oldWidth-clippedWidth)/bottomImage.width
                            imageRect.cropVoffset = oldVoffset - 0.5*(oldHeight-clippedHeight)/bottomImage.height
                        }
                    }
                    onReleased: {
                        if (!containsMouse) {
                            handleVisible = false
                        }
                        imageRect.aspectText = ""
                        preventStealing = false
                        imageRect.validateCrop()
                        cropDrag.updatePosition()
                    }
                }
                MouseArea {
                    id: cropResizeBottomRight
                    acceptedButtons: Qt.LeftButton
                    hoverEnabled: true
                    enabled: cropDrag.enabled
                    visible: cropDrag.visible
                    width: imageRect.cropHandleWidth*uiScale/bottomImage.scale
                    height: imageRect.cropHandleWidth*uiScale/bottomImage.scale
                    x: bottomImage.x + Math.round(0.5*(bottomImage.width)*bottomImage.scale + (cropDrag.hoffset + cropDrag.width/(2*bottomImage.width))*bottomImage.width*bottomImage.scale)
                    y: bottomImage.y + Math.round(0.5*(bottomImage.height)*bottomImage.scale + (cropDrag.voffset + cropDrag.height/(2*bottomImage.height))*bottomImage.height*bottomImage.scale)
                    transform: Scale {
                        origin.x: 0
                        origin.y: 0
                        xScale: bottomImage.scale
                        yScale: bottomImage.scale
                    }

                    property real oldX
                    property real oldY
                    property real oldWidth
                    property real oldHeight
                    property real unclippedWidth
                    property real unclippedHeight
                    property real clippedWidth
                    property real clippedHeight
                    property real oldHoffset
                    property real oldVoffset
                    property real oldAspect
                    property real lockedAspect

                    property bool handleVisible: false
                    onEntered: {
                        handleVisible = true
                    }
                    onExited: {
                        if (!pressed) {
                            handleVisible = false
                        }
                    }
                    onPressed: {
                        imageRect.validateCrop()
                        preventStealing = true
                        oldX = mouse.x
                        oldY = mouse.y
                        oldWidth = cropmarker.width
                        oldHeight = cropmarker.height
                        unclippedWidth = cropmarker.width
                        unclippedHeight = cropmarker.height
                        oldHoffset = cropmarker.hoffset
                        oldVoffset = cropmarker.voffset
                        oldAspect = cropmarker.width/cropmarker.height
                        lockedAspect = oldAspect
                    }
                    onPositionChanged: {
                        if (pressed) {
                            var deltaX = mouse.x - oldX
                            var deltaY = mouse.y - oldY
                            oldX = mouse.x
                            oldY = mouse.y
                            unclippedWidth = unclippedWidth + deltaX
                            unclippedHeight = unclippedHeight + deltaY
                            var clippedWidth = Math.round(Math.min(Math.max(1,unclippedWidth),bottomImage.width*(0.5-oldHoffset)+0.5*oldWidth))
                            var clippedHeight = Math.round(Math.min(Math.max(1, unclippedHeight), bottomImage.height*(0.5-oldVoffset)+0.5*oldHeight))
                            var newAspect = clippedWidth/clippedHeight
                            if (mouse.modifiers & Qt.ShiftModifier && !(mouse.modifiers & Qt.ControlModifier)) {
                                //set aspect to a snapped one
                                if      (newAspect < 0.3478) {lockedAspect = 1/3;    imageRect.aspectText = "1:3"}
                                else if (newAspect < 0.3936) {lockedAspect = 1/2.76; imageRect.aspectText = "1:2.76"}
                                else if (newAspect < 0.4622) {lockedAspect = 1/2.35; imageRect.aspectText = "1:2.35"}
                                else if (newAspect < 0.5308) {lockedAspect = 1/2;    imageRect.aspectText = "1:2"}
                                else if (newAspect < 0.5899) {lockedAspect = 9/16;   imageRect.aspectText = "9:16"}
                                else if (newAspect < 0.6420) {lockedAspect = 0.61803;imageRect.aspectText = "1:φ"}
                                else if (newAspect < 0.7075) {lockedAspect = 2/3;    imageRect.aspectText = "2:3"}
                                else if (newAspect < 0.7747) {lockedAspect = 3/4;    imageRect.aspectText = "3:4"}
                                else if (newAspect < 0.8950) {lockedAspect = 4/5;    imageRect.aspectText = "4:5"}
                                else if (newAspect < 1.1173) {lockedAspect = 1;      imageRect.aspectText = "1:1"}
                                else if (newAspect < 1.2908) {lockedAspect = 5/4;    imageRect.aspectText = "5:4"}
                                else if (newAspect < 1.4134) {lockedAspect = 4/3;    imageRect.aspectText = "4:3"}
                                else if (newAspect < 1.5574) {lockedAspect = 3/2;    imageRect.aspectText = "3:2"}
                                else if (newAspect < 1.6951) {lockedAspect = 1.61803;imageRect.aspectText = "φ:1"}
                                else if (newAspect < 1.8837) {lockedAspect = 16/9;   imageRect.aspectText = "16:9"}
                                else if (newAspect < 2.1634) {lockedAspect = 2;      imageRect.aspectText = "2:1"}
                                else if (newAspect < 2.5407) {lockedAspect = 2.35;   imageRect.aspectText = "2.35:1"}
                                else if (newAspect < 2.8755) {lockedAspect = 2.76;   imageRect.aspectText = "2.76:1"}
                                else                         {lockedAspect = 3;      imageRect.aspectText = "3:1"}
                            }

                            if (mouse.modifiers & Qt.ControlModifier || mouse.modifiers & Qt.ShiftModifier) {
                                //choose whether to use height or width based on aspect, then clip them
                                if (lockedAspect < newAspect) {//narrower, we adjust the height
                                    //Set both width and height explicitly so that the offsets can be corrected later.
                                    clippedHeight = Math.round(Math.min(Math.max(1, clippedWidth/lockedAspect),bottomImage.height*(0.5-oldVoffset)+0.5*oldHeight))
                                    clippedWidth = clippedHeight*lockedAspect
                                    imageRect.cropHeight = clippedHeight/bottomImage.height
                                } else {
                                    clippedWidth = Math.round(Math.min(Math.max(1,clippedHeight*lockedAspect),bottomImage.width*(0.5-oldHoffset)+0.5*oldWidth))
                                    clippedHeight = clippedWidth/lockedAspect
                                    imageRect.cropHeight = clippedHeight/bottomImage.height
                                }
                                //then set aspect back to the original in case of snapping.
                                imageRect.cropAspect = lockedAspect
                            } else {
                                lockedAspect = newAspect
                                imageRect.cropHeight = clippedHeight/bottomImage.height
                                imageRect.cropAspect = newAspect
                            }
                            //Now we want to remember where the right edge of the image was, and preserve that.
                            imageRect.cropHoffset = oldHoffset - 0.5*(oldWidth-clippedWidth)/bottomImage.width
                            imageRect.cropVoffset = oldVoffset - 0.5*(oldHeight-clippedHeight)/bottomImage.height
                        }
                    }
                    onReleased: {
                        if (!containsMouse) {
                            handleVisible = false
                        }
                        imageRect.aspectText = ""
                        preventStealing = false
                        imageRect.validateCrop()
                        cropDrag.updatePosition()
                    }
                }
            }
        }
        MouseArea {
            id: wheelCapture
            anchors.fill: flicky
            acceptedButtons: Qt.NoButton//Qt.RightButton
            onWheel: {
                var oldMouseX = wheel.x + flicky.contentX - Math.max(0, 0.5*(flicky.width  - bottomImage.width*bottomImage.scale))  - Math.floor(cropMargin*uiScale*cropping)
                var oldMouseY = wheel.y + flicky.contentY - Math.max(0, 0.5*(flicky.height - bottomImage.height*bottomImage.scale)) - Math.floor(cropMargin*uiScale*cropping)
                //1.2 is the zoom factor for a normal wheel click, and 120 units is the 'angle' of a normal wheel click.
                var zoomFactor = Math.pow(1.2,Math.abs(wheel.angleDelta.y)/120)
                if (wheel.angleDelta.y > 0) {
                    bottomImage.scale *= zoomFactor;
                    flicky.contentX = oldMouseX*zoomFactor - wheel.x + Math.max(0, 0.5*(flicky.width  - bottomImage.width*bottomImage.scale))  + Math.floor(cropMargin*uiScale*cropping)
                    flicky.contentY = oldMouseY*zoomFactor - wheel.y + Math.max(0, 0.5*(flicky.height - bottomImage.height*bottomImage.scale)) + Math.floor(cropMargin*uiScale*cropping)
                    //For cropping, we don't want any surprise motions.
                    if (root.cropping) {
                        flicky.returnToBounds()
                    }
                }
                else {
                    bottomImage.scale /= zoomFactor;
                    flicky.contentX = oldMouseX/zoomFactor - wheel.x + Math.max(0, 0.5*(flicky.width  - bottomImage.width*bottomImage.scale))  + Math.floor(cropMargin*uiScale*cropping)
                    flicky.contentY = oldMouseY/zoomFactor - wheel.y + Math.max(0, 0.5*(flicky.height - bottomImage.height*bottomImage.scale)) + Math.floor(cropMargin*uiScale*cropping)
                    flicky.returnToBounds()
                }
                if (bottomImage.scale == flicky.fitScale) {flicky.fit = true}
                else {flicky.fit = false}
            }
        }

        Item {
            id: backgroundColorBox
            y: 0 * uiScale
            width: 180 * uiScale
            height: 30 * uiScale
            anchors.right: crop.left
            Text {
                id: backgroundColorText
                x: 0 * uiScale
                y: 4 * uiScale
                width: parent.width
                color: "white"
                text: qsTr("Background Brightness")
                horizontalAlignment: Text.AlignHCenter
                font.pixelSize: 12.0 * uiScale
            }

            SlipperySlider {
                id: backgroundBrightnessSlider
                x: 40 * uiScale
                y: 18 * uiScale
                width: parent.width - 80 * uiScale
                minimumValue: 0
                maximumValue: 2
                value: 0
                stepSize: 1
                tickmarksEnabled: true
                uiScale: root.uiScale
            }
        }

        ToolButton {
            id: crop
            anchors.right: rotateLeft.left
            y: 0 * uiScale
            width: 120 * uiScale
            notDisabled: root.imageReady
            text: root.cropping ? qsTr("Finish Crop") : qsTr("Crop")//Change to "Adjust crop" when a crop exists; change to "Accept crop" when cropping in progress
            tooltipText: root.cropping ? qsTr("Click this to save your crop."): qsTr("Click this to begin cropping.")//change to "Hold shift to snap to common aspect ratios" when cropping in progress
            onTriggered: {
                if (!root.cropping) {
                    filmProvider.disableThumbnailWrite()
                    root.requestingCropping = true
                } else {
                    filmProvider.enableThumbnailWrite()
                    root.cancelCropping = false
                    root.requestingCropping = false
                }
            }
            Component.onCompleted: {
                crop.tooltipWanted.connect(root.tooltipWanted)
            }
            uiScale: root.uiScale
        }

        ToolButton {
            id: rotateLeft
            anchors.right: rotateRight.left
            y: 0 * uiScale
            width: 90 * uiScale
            text: qsTr("Rotate Left")
            onTriggered: {
                paramManager.rotateLeft()
                root.updateImage()
            }
            uiScale: root.uiScale
        }

        ToolButton {
            id: rotateRight
            anchors.right: fitscreen.left
            y: 0 * uiScale
            width: 90 * uiScale
            text: qsTr("Rotate Right")
            onTriggered: {
                paramManager.rotateRight()
                root.updateImage()
            }
            uiScale: root.uiScale
        }

        ToolButton {
            id: fitscreen
            anchors.right: fullzoom.left
            y: 0 * uiScale
            text: qsTr("Fit")
            onTriggered: {
                if(bottomImage.width != 0 && bottomImage.height != 0) {
                    bottomImage.scale = flicky.fitScale
                }
                else {
                    bottomImage.scale = 1
                }
                flicky.returnToBounds()
                flicky.fit = true
            }
            uiScale: root.uiScale
        }
        ToolButton {
            id: fullzoom
            anchors.right: zoomin.left
            y: 0 * uiScale
            text: "1:1"
            onTriggered: {
                var oldCenterX = flicky.centerX
                var oldCenterY = flicky.centerY
                bottomImage.scale = 1
                flicky.contentX = oldCenterX*1 -  bottomImage.width*Math.min(1, flicky.fitScaleX) / 2
                flicky.contentY = oldCenterY*1 - bottomImage.height*Math.min(1, flicky.fitScaleY) / 2
                if (bottomImage.scale == flicky.fitScale){flicky.fit = true}
                else {flicky.fit = false}
            }
            uiScale: root.uiScale
        }
        ToolButton {
            id: zoomin
            anchors.right: zoomout.left
            y: 0 * uiScale
            text: "+"
            onTriggered: {
                var oldCenterX = flicky.centerX
                var oldCenterY = flicky.centerY
                bottomImage.scale *= 1.2
                flicky.contentX = oldCenterX*bottomImage.scale -  bottomImage.width*Math.min(bottomImage.scale, flicky.fitScaleX) / 2
                flicky.contentY = oldCenterY*bottomImage.scale - bottomImage.height*Math.min(bottomImage.scale, flicky.fitScaleY) / 2
                if (bottomImage.scale == flicky.fitScale){flicky.fit = true}
                else {flicky.fit = false}
            }
            uiScale: root.uiScale
        }
        ToolButton {
            id: zoomout
            anchors.right: parent.right
            y: 0 * uiScale
            text: "-"
            onTriggered: {
                var oldCenterX = flicky.centerX
                var oldCenterY = flicky.centerY
                bottomImage.scale /= 1.2
                var tempScale = Math.max(bottomImage.scale, flicky.fitScale)
                flicky.contentX = oldCenterX*tempScale -  bottomImage.width*Math.min(tempScale, flicky.fitScaleX) / 2
                flicky.contentY = oldCenterY*tempScale - bottomImage.height*Math.min(tempScale, flicky.fitScaleY) / 2
                flicky.returnToBounds()
                if (bottomImage.scale == flicky.fitScale) {flicky.fit = true}
                else {flicky.fit = false}
            }
            uiScale: root.uiScale
        }
        FilmProgressBar {
            id: progressBar
            visible: true
            value: filmProvider.progress
            Connections {
                target: filmProvider
                onProgressChanged: progressBar.value = filmProvider.progress
            }
            uiScale: root.uiScale
        }
        Text {
            id: apertureText
            x: 200 * uiScale
            y: 0 * uiScale
            color: "white"
            text: root.cropping ? qsTr("Width: ") + imageRect.displayWidth : " f/" + paramManager.aperture
            font.pixelSize: 12.0 * uiScale
            elide: Text.ElideRight
        }
        Text {
            id: shutterText
            x: 200 * uiScale
            y: 15 * uiScale
            color: "white"
            text: root.cropping ? qsTr("Height: ") + imageRect.displayHeight : " " + paramManager.exposureTime + " s"
            font.pixelSize: 12.0 * uiScale
            elide: Text.ElideRight
        }
        Text {
            id: filenameText
            //x: 300 * uiScale
            x: 350 * uiScale
            y: 0 * uiScale
            color: "white"
            text: root.cropping ? qsTr("H offset: ") + imageRect.displayHoffset : paramManager.filename
            font.pixelSize: 12.0 * uiScale
            elide: Text.ElideRight
        }
        Text {
            id: isoText
            //x: 300 * uiScale
            x: 350 * uiScale
            y: 15 * uiScale
            color: "white"
            text: root.cropping ? qsTr("V offset: ") + imageRect.displayVoffset : "ISO " + paramManager.sensitivity
            font.pixelSize: 12.0 * uiScale
            elide: Text.ElideRight
        }
    }
    EditTools {
        id: editTools
        uiScale: root.uiScale
        imageReady: root.imageReady
        cropping: root.requestingCropping || root.cropping
        Component.onCompleted: {
            editTools.tooltipWanted.connect(root.tooltipWanted)
        }
    }
}
