import QtQuick 2.12
import QtQuick.Layouts 1.12
import QtQuick.Window 2.12
import "gui_components"
import "colors.js" as Colors

SlimSplitView {
    id: root
    Layout.fillWidth: true
    Layout.fillHeight: true
    orientation: Qt.Horizontal
    property real uiScale: 1
    property bool imageReady: false//must only be made ready when the full size image is ready
    property bool previewReady: false//must be made true when preview OR full size is ready
    property bool noImage: true//gets turned false once any image is loaded, and stays false

    property bool requestingCropping: false
    property bool cropping: false
    property int cropMarkFlashCount: 0
    property bool cropMarkFlash: false
    property bool cancelCropping: false
    property real cropMargin: 50//200

    property bool requestingLeveling: false
    property bool leveling: false
    property bool cancelLeveling: false
    property real tempCropHeight: 0

    property bool wbPicking: false

    property bool onEditTab

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
                paramManager.cropHeight = 0 //signal to image pipeline to disable cropping
            }
        } else {//we're done cropping
            if (!cancelCropping) {//accepted the crop
                if (imageRect.noCrop) {
                    //if the crop is the full image, we want to set it to 0
                    //if we don't, if you rotate the image 90 degrees it keeps the aspect ratio
                    paramManager.cropHeight = 0
                    paramManager.cropAspect = 0
                    paramManager.cropVoffset = 0
                    paramManager.cropHoffset = 0
                } else {//the crop isn't the entire image
                    paramManager.cropHeight = imageRect.readHeight
                    paramManager.cropAspect = imageRect.readAspect
                    paramManager.cropVoffset = imageRect.readVoffset
                    paramManager.cropHoffset = imageRect.readHoffset
                }
                //send stuff back to database
                paramManager.writeback()
            } else { //canceling crop, so no writeback
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
        if (cropping) {
            //Flash the crop mark
            cropMarkFlash = true
            cropMarkTimer.start()
        } else {
            cropMarkTimer.stop()
            cropMarkFlash = false
            cropMarkFlashCount = 0
        }
    }
    Timer {
        id: cropMarkTimer
        interval: 250
        onTriggered: {
            if (cropMarkFlashCount < 3) {
                cropMarkFlash = !cropMarkFlash
                cropMarkFlashCount += 1
                cropMarkTimer.restart()
            }
        }
    }

    onRequestingLevelingChanged: {
        if (requestingLeveling == true) {
            //set up the leveling variables
            imageRect.rotationAngle = paramManager.rotationAngle
            if (paramManager.rotationPointX >= 0 || paramManager.rotationPointY >= 0) {//the database has a point already
                rotationDrag.notClickedYet = false
            } else {
                rotationDrag.notClickedYet = true //have it follow the cursor around until it's clicked
            }

            imageRect.readRotationPointX = paramManager.rotationPointX
            imageRect.readRotationPointY = paramManager.rotationPointY
            if (paramManager.rotationAngle == 0) {
                //we don't need to update the image, so just turn on cropping
                leveling = true
            } else { //we need to disable rotation; -50 will indicate to the pipeline that this is disabled.
                paramManager.rotationAngle = -50
                //when the image is ready, then cropping will be set to true by topImage
            }
            root.tempCropHeight = paramManager.cropHeight
            paramManager.cropHeight = 0
        } else {//we're done leveling
            if (!cancelLeveling) {//accepted the leveling
                paramManager.rotationPointX = imageRect.readRotationPointX
                paramManager.rotationPointY = imageRect.readRotationPointY
                paramManager.rotationAngle = imageRect.rotationAngle
                paramManager.cropHeight = root.tempCropHeight
                paramManager.writeback()
            } else {
                cancelLeveling = false
            }
        }
    }

    onLevelingChanged: {
        flicky.returnToBounds()
    }

    signal tooltipWanted(string text, int x, int y)

    //This is for telling the queue the latest image source so it can show it until the thumb updates.
    signal imageURL(string newURL)


    Rectangle {
        id: photoBox
        color: "black"
        Layout.fillWidth: true
        property int backgroundColor: 0
        property bool loadingError: false
        property string errorText: ""
        Rectangle {//This is because before the image is loaded there's no background.
            id: background
            x: 0 * uiScale
            y: Math.ceil(30 * uiScale)
            width: parent.width
            height: Math.floor(parent.height - 30 * uiScale)
            color: photoBox.backgroundColor == 2 ? "white" : photoBox.backgroundColor == 1 ? "gray" : "black"

            Rectangle {
                id: noPictureBox
                width: 400 * uiScale
                height: noPictureText.contentHeight + 40 * uiScale
                anchors.centerIn: parent
                color: Colors.darkGray
                border.color: Colors.lowGray
                border.width: 2 * uiScale
                radius: 10 * uiScale
                visible: root.noImage

                Text {
                    id: noPictureText
                    width: 350 * uiScale
                    anchors.centerIn: parent
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                    color: "white"
                    font.pixelSize: 14.0 * uiScale
                    wrapMode: Text.Wrap
                    text: qsTr("Select an image to edit by double-clicking on a photo in the Work Queue. Switch between images with the right and left arrow keys.\n\nRate the current image by pressing 0 through 5 and X, or by pressing up or down arrow keys.")
                }
            }
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

                    Connections {
                        target: settings
                        function onMipmapViewChanged() { topImage.mipmap = settings.getMipmapView() }
                    }

                    property string state: "nl"//not loaded

                    Connections {
                        target: paramManager
                        function onImageIndexChanged() {
                            //this happens when paramManager.selectImage is performed and the selected image changed
                            topImage.state = "lt"//loading thumbnail
                            //selectImage still emits update image via paramChargeWrapper so we don't need to do any more
                        }
                        function onUpdateImage(newImage) {
                            if (newImage) {//If this comes from paramManager.selectImage, then we want to cancel crop or leveling
                                cancelCropping = true
                                requestingCropping = false
                                cancelLeveling = true
                                requestingLeveling = false
                                wbPicking = false
                            }

                            //Irrespective of that
                            if (topImage.state == "lt") {//a NEW image has been selected
                                //load thumbnail into top image
                                var thumbPath = (Qt.platform.os == "windows" ? 'file:///' : 'file://') + organizeModel.thumbDir() + '/' + paramManager.imageIndex.slice(0,4) + '/' + paramManager.imageIndex + '.jpg'
                                topImage.source = thumbPath
                            }
                            else {//not a new image; probably just a slider move
                                //Increment the image index
                                var num = (topImage.index + 1) % 1000000//1 in a million
                                topImage.index = num;
                                var s = num+"";
                                var size = 6 //6 digit number
                                while (s.length < size) {s = "0" + s}
                                topImage.indexString = s

                                //now actually ask for the image
                                if (settings.getQuickPreview()) {//load the quick pipe
                                    topImage.state = "lq"//loading quick pipe
                                    topImage.source = "image://filmy/q" + topImage.indexString
                                }
                                else {//load the full size image
                                    topImage.state = "lf"// loading full image
                                    topImage.source = "image://filmy/f" + topImage.indexString
                                }
                            }
                        }
                    }

                    onStatusChanged: {
                        var num
                        var s
                        var size
                        console.log("topImage state: " + topImage.state)
                        console.log("topImage status: " + topImage.status)
                        if (topImage.status == Image.Ready) { //if the image is now ready
                            //First, we turn off the no-image-text
                            root.noImage = false
                            //Next, we copy to the bottom image, regardless of what else.
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


                            if (topImage.state == "lf") {//it was loading the full image
                                topImage.state = "sf"//showing full image
                                root.imageReady = true
                                root.previewReady = true
                                root.imageURL(topImage.source)//replace the thumbnail in the queue with the live image
                                filmProvider.writeThumbnail(paramManager.imageIndex)
                            }
                            else {//it was loading the thumb or the quick image
                                root.imageReady = false
                                //Increment the image index
                                num = (topImage.index + 1) % 1000000//1 in a million
                                topImage.index = num;
                                s = num+"";
                                size = 6 //6 digit number
                                while (s.length < size) {s = "0" + s}
                                topImage.indexString = s

                                //now actually ask for the image
                                if (topImage.state == "lt") {//it was loading the thumbnail
                                    topImage.state = "lq"//loading quick pipe
                                    root.previewReady = false
                                    topImage.source = "image://filmy/q" + topImage.indexString
                                }
                                else if (topImage.state == "lq") {//it was loading the quick image
                                    topImage.state = "lf"//loading full image
                                    root.previewReady = true
                                    topImage.source = "image://filmy/f" + topImage.indexString
                                }
                            }

                            //When the image is ready, we want to process cropping.
                            //Cropping should only be enabled when the state is 'lq' or 'lf'...
                            // but if it's 'lt' then it's already not going to be cropping.
                            if (root.requestingCropping) {
                                root.cropping = true
                            } else {
                                root.cropping = false
                            }
                            //Same for leveling
                            if (root.requestingLeveling) {
                                root.leveling = true
                            } else {
                                root.leveling = false
                            }
                        }
                        else if (photoBox.loadingError) {//the source file is not available
                            //This needed to be added in case you click on an unavailable image before any image is loaded
                            root.imageReady = false
                            console.log("file is unavailable")
                            //doing nothing here seems to work fine
                        }
                        else if (topImage.status == Image.Error) {
                            root.imageReady = false
                            console.log("top image errored")
                            //Increment the image index
                            num = (topImage.index + 1) % 1000000//1 in a million
                            topImage.index = num;
                            s = num+"";
                            size = 6 //6 digit number
                            while (s.length < size) {s = "0" + s}
                            topImage.indexString = s

                            //now actually ask for the image
                            //we always go back to loading quick even when the full image fails
                            topImage.state = "lq"//loading quick pipe
                            topImage.source = "image://filmy/q" + topImage.indexString
                        }
                        console.log("topImage state became: " + topImage.state)
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
                            console.log("bottom image ready")
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
                        function onMipmapViewChanged() { bottomImage.mipmap = settings.getMipmapView() }
                    }
                }
                MouseArea {
                    id: doubleClickCapture
                    anchors.fill: parent
                    hoverEnabled: true
                    acceptedButtons: Qt.LeftButton
                    onDoubleClicked: {
                        if (bottomImage.scale < flicky.fitScale || bottomImage.scale === 1/Screen.devicePixelRatio) {
                            bottomImage.scale = flicky.fitScale
                            flicky.contentX = Math.floor(cropMargin*uiScale*cropping)
                            flicky.contentY = Math.floor(cropMargin*uiScale*cropping)
                            flicky.fit = true
                        } else {
                            var nativeZoom = 1/Screen.devicePixelRatio
                            var zoomFactor = nativeZoom/bottomImage.scale

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

                            bottomImage.scale = nativeZoom

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
                            function onAspectTextChanged() {
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
                    visible: root.cropping && (cropResizeLeft.handleVisible || root.cropMarkFlash)
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
                    visible: root.cropping && (cropResizeRight.handleVisible || root.cropMarkFlash)
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
                    visible: root.cropping && (cropResizeTop.handleVisible || root.cropMarkFlash)
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
                    visible: root.cropping && (cropResizeBottom.handleVisible || root.cropMarkFlash)
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
                    visible: root.cropping && (cropResizeTopLeft.handleVisible || cropmarker.tooSmall || root.cropMarkFlash)
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
                    visible: root.cropping && (cropResizeTopRight.handleVisible || cropmarker.tooSmall || root.cropMarkFlash)
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
                    visible: root.cropping && (cropResizeBottomLeft.handleVisible || cropmarker.tooSmall || root.cropMarkFlash)
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
                    visible: root.cropping && (cropResizeBottomRight.handleVisible || cropmarker.tooSmall || root.cropMarkFlash)
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
                //Readout for checking whether the crop is the entire image or not
                property bool noCrop: cropmarker.height===bottomImage.height && cropmarker.width===bottomImage.width
                //For showing on the screen
                property real displayWidth:  cropmarker.width
                property real displayHeight: cropmarker.height
                property real displayHoffset: 0.5 * Math.round(2 * cropmarker.hoffset * bottomImage.width)
                property real displayVoffset: 0.5 * Math.round(2 * cropmarker.voffset * bottomImage.height)
                //aspect ratio text
                property string aspectText: ""
                //Constant; minimum crop size
                property int minCrop: 20

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
                        function onCroppingChanged() {
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
                        function onImageReadyChanged() {
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
                                var clippedWidth = Math.round(Math.min(Math.max(imageRect.minCrop,unclippedWidth),bottomImage.width*(0.5+oldOffset)+0.5*oldWidth))
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
                                var clippedWidth = Math.round(Math.min(Math.max(imageRect.minCrop,unclippedWidth),bottomImage.width*(0.5-oldOffset)+0.5*oldWidth))
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
                                var clippedHeight = Math.round(Math.min(Math.max(imageRect.minCrop, unclippedHeight), bottomImage.height*(0.5+oldOffset)+0.5*oldHeight))
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
                                var clippedHeight = Math.round(Math.min(Math.max(imageRect.minCrop, unclippedHeight), bottomImage.height*(0.5-oldOffset)+0.5*oldHeight))
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
                            var clippedWidth = Math.round(Math.min(Math.max(imageRect.minCrop, unclippedWidth),bottomImage.width*(0.5+oldHoffset)+0.5*oldWidth))
                            var clippedHeight = Math.round(Math.min(Math.max(imageRect.minCrop, unclippedHeight), bottomImage.height*(0.5+oldVoffset)+0.5*oldHeight))
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
                                    clippedHeight = Math.round(Math.min(Math.max(imageRect.minCrop, clippedWidth/lockedAspect),bottomImage.height*(0.5+oldVoffset)+0.5*oldHeight))
                                    clippedWidth = clippedHeight*lockedAspect
                                    imageRect.cropHeight = clippedHeight/bottomImage.height
                                } else {
                                    clippedWidth = Math.round(Math.min(Math.max(imageRect.minCrop, clippedHeight*lockedAspect),bottomImage.width*(0.5+oldHoffset)+0.5*oldWidth))
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
                            var clippedWidth = Math.round(Math.min(Math.max(imageRect.minCrop, unclippedWidth),bottomImage.width*(0.5-oldHoffset)+0.5*oldWidth))
                            var clippedHeight = Math.round(Math.min(Math.max(imageRect.minCrop, unclippedHeight), bottomImage.height*(0.5+oldVoffset)+0.5*oldHeight))
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
                                    clippedHeight = Math.round(Math.min(Math.max(imageRect.minCrop, clippedWidth/lockedAspect),bottomImage.height*(0.5+oldVoffset)+0.5*oldHeight))
                                    clippedWidth = clippedHeight*lockedAspect
                                    imageRect.cropHeight = clippedHeight/bottomImage.height
                                } else {
                                    clippedWidth = Math.round(Math.min(Math.max(imageRect.minCrop, clippedHeight*lockedAspect),bottomImage.width*(0.5-oldHoffset)+0.5*oldWidth))
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
                            var clippedWidth = Math.round(Math.min(Math.max(imageRect.minCrop, unclippedWidth),bottomImage.width*(0.5+oldHoffset)+0.5*oldWidth))
                            var clippedHeight = Math.round(Math.min(Math.max(imageRect.minCrop, unclippedHeight), bottomImage.height*(0.5-oldVoffset)+0.5*oldHeight))
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
                                    clippedHeight = Math.round(Math.min(Math.max(imageRect.minCrop, clippedWidth/lockedAspect),bottomImage.height*(0.5-oldVoffset)+0.5*oldHeight))
                                    clippedWidth = clippedHeight*lockedAspect
                                    imageRect.cropHeight = clippedHeight/bottomImage.height
                                } else {
                                    clippedWidth = Math.round(Math.min(Math.max(imageRect.minCrop, clippedHeight*lockedAspect),bottomImage.width*(0.5+oldHoffset)+0.5*oldWidth))
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
                            var clippedWidth = Math.round(Math.min(Math.max(imageRect.minCrop, unclippedWidth),bottomImage.width*(0.5-oldHoffset)+0.5*oldWidth))
                            var clippedHeight = Math.round(Math.min(Math.max(imageRect.minCrop, unclippedHeight), bottomImage.height*(0.5-oldVoffset)+0.5*oldHeight))
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
                                    clippedHeight = Math.round(Math.min(Math.max(imageRect.minCrop, clippedWidth/lockedAspect),bottomImage.height*(0.5-oldVoffset)+0.5*oldHeight))
                                    clippedWidth = clippedHeight*lockedAspect
                                    imageRect.cropHeight = clippedHeight/bottomImage.height
                                } else {
                                    clippedWidth = Math.round(Math.min(Math.max(imageRect.minCrop, clippedHeight*lockedAspect),bottomImage.width*(0.5-oldHoffset)+0.5*oldWidth))
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

                //Leveling tool stuff.

                //We'll store one point relative to the image boundaries, and the angle.
                //The point will be defined relative to the image width and height.
                // We don't care if it moves somewhere unrelated when copy/pasting between images.
                // When you rotate the image by 90 degrees, it'll move with the image.
                //The angle is going to be restricted to between -45 and +45 degrees.
                property real rotationAngle
                property real readRotationPointX
                property real readRotationPointY
                property real displayRotationPointX: readRotationPointX*bottomImage.width
                property real displayRotationPointY: readRotationPointY*bottomImage.height
                //When you rotate farther it'll modulo back.

                //Next is the visible stuff.
                //We make them invisible when the coordinates are negative
                Rectangle {
                    id: verticalAngleMark
                    width: 1
                    height: 2 * Math.max(Math.max(bottomImage.width, bottomImage.height)*bottomImage.scale,imageRect.height)
                    x: bottomImage.x + imageRect.displayRotationPointX*bottomImage.scale - width/2
                    y: bottomImage.y + imageRect.displayRotationPointY*bottomImage.scale - height/2
                    color: (rotationDrag.overCross && !rotationDrag.centering) ? Colors.medOrange : photoBox.backgroundColor == 2 ? "black" : photoBox.backgroundColor == 1 ? "gray" : "white"
                    rotation: imageRect.rotationAngle
                    visible: root.leveling && (imageRect.readRotationPointX >= 0) && (imageRect.readRotationPointY >= 0) && (root.imageReady || root.previewReady) && root.requestingLeveling
                }
                Rectangle {
                    id: horizontalAngleMark
                    width: 2 * Math.max(Math.max(bottomImage.width, bottomImage.height)*bottomImage.scale,imageRect.width)
                    height: 1
                    x: bottomImage.x + imageRect.displayRotationPointX*bottomImage.scale - width/2
                    y: bottomImage.y + imageRect.displayRotationPointY*bottomImage.scale - height/2
                    color: (rotationDrag.overCross && !rotationDrag.centering) ? Colors.medOrange : photoBox.backgroundColor == 2 ? "black" : photoBox.backgroundColor == 1 ? "gray" : "white"
                    rotation: imageRect.rotationAngle
                    visible: root.leveling && (imageRect.readRotationPointX >= 0) && (imageRect.readRotationPointY >= 0) && (root.imageReady || root.previewReady) && root.requestingLeveling
                }
                Rectangle {
                    id: rotationCenterMark
                    width: 40 * uiScale
                    height: width
                    x: bottomImage.x + imageRect.displayRotationPointX*bottomImage.scale - width/2
                    y: bottomImage.y + imageRect.displayRotationPointY*bottomImage.scale - height/2
                    radius: width/2
                    color: "#00000000"
                    border.width: 2 * uiScale
                    border.color: Colors.medOrange
                    visible: root.leveling && (rotationDrag.overPoint || rotationDrag.centering) && (root.imageReady || root.previewReady) && root.requestingLeveling
                }

                MouseArea {
                    id: rotationDrag
                    acceptedButtons: Qt.LeftButton | Qt.RightButton
                    width: bottomImage.width
                    height: bottomImage.height
                    x: bottomImage.x
                    y: bottomImage.y
                    transform: Scale {
                        origin.x: 0
                        origin.y: 0
                        xScale: bottomImage.scale
                        yScale: bottomImage.scale
                    }

                    enabled: root.leveling && (root.imageReady || root.previewReady) && root.requestingLeveling
                    visible: root.leveling && (root.imageReady || root.previewReady) && root.requestingLeveling
                    hoverEnabled: true
                    preventStealing: false//root.leveling && root.imageReady

                    property bool notClickedYet: true
                    property bool centering: false
                    property bool rotating: false
                    property bool overPoint: false
                    property bool overCross: false
                    property real oldAngle: 0
                    property real oldX: 0
                    property real oldY: 0
                    property real tempRotationPointX: -1
                    property real tempRotationPointY: -1

                    property real oldImageWidth: 1
                    property real oldImageHeight: 1

                    onPressed: {
                        if (pressedButtons & Qt.LeftButton) {
                            flicky.returnToBounds()
                            if (notClickedYet) { //no center yet
                                notClickedYet = false
                                imageRect.readRotationPointX = mouse.x / bottomImage.width
                                imageRect.readRotationPointY = mouse.y / bottomImage.height
                                preventStealing = true
                            } else { //the center has already been defined
                                var distanceX = (mouse.x - imageRect.displayRotationPointX)*bottomImage.scale
                                var distanceY = (mouse.y - imageRect.displayRotationPointY)*bottomImage.scale
                                if (overPoint) {//drag the rotation reference point if it's close enough
                                    centering = true
                                    rotating = false
                                    oldX = mouse.x
                                    oldY = mouse.y
                                    tempRotationPointX = imageRect.readRotationPointX
                                    tempRotationPointY = imageRect.readRotationPointY
                                    preventStealing = true
                                } else if (overCross){ //rotation
                                    centering = false
                                    rotating = true
                                    oldAngle = Math.atan2(mouse.y-imageRect.displayRotationPointY, mouse.x-imageRect.displayRotationPointX)*180/Math.PI
                                    preventStealing = true
                                }
                            }
                        }
                    }
                    onPositionChanged: {
                        if (notClickedYet) { //display the cursors where the mouse is
                            imageRect.readRotationPointX = mouse.x / bottomImage.width
                            imageRect.readRotationPointY = mouse.y / bottomImage.height
                        } else {
                            if (rotationDrag.pressed && (pressedButtons & Qt.LeftButton)) {
                                if (centering) { //drag the rotation reference point
                                    var deltaX = mouse.x - oldX
                                    var deltaY = mouse.y - oldY
                                    tempRotationPointX += deltaX / bottomImage.width
                                    tempRotationPointY += deltaY / bottomImage.height
                                    imageRect.readRotationPointX = Math.max(0, Math.min(1, tempRotationPointX))
                                    imageRect.readRotationPointY = Math.max(0, Math.min(1, tempRotationPointY))
                                    oldX = mouse.x
                                    oldY = mouse.y
                                } else if (rotating) {
                                    var newAngle = Math.atan2(mouse.y-imageRect.displayRotationPointY, mouse.x-imageRect.displayRotationPointX)*180/Math.PI
                                    var delta = newAngle - oldAngle
                                    imageRect.rotationAngle += delta
                                    if (imageRect.rotationAngle > 45) { //more than 45 degrees either way
                                        imageRect.rotationAngle -= 90
                                    } else if (imageRect.rotationAngle < -45) {
                                        imageRect.rotationAngle += 90
                                    }

                                    oldAngle = newAngle
                                }
                            }
                            var distanceX = (mouse.x - imageRect.displayRotationPointX)*bottomImage.scale
                            var distanceY = (mouse.y - imageRect.displayRotationPointY)*bottomImage.scale
                            var radius = Math.sqrt(distanceX*distanceX + distanceY*distanceY)
                            var mouseAngle = Math.atan2(mouse.y-imageRect.displayRotationPointY, mouse.x-imageRect.displayRotationPointX)*180/Math.PI
                            if (mouseAngle > 90) {
                                mouseAngle -= 90
                            }
                            if (mouseAngle < -90) {
                                mouseAngle += 90
                            }
                            if (mouseAngle > 45) {
                                mouseAngle -= 90
                            }
                            if (mouseAngle < -45) {
                                mouseAngle += 90
                            }
                            var deltaAngle = Math.abs(mouseAngle - imageRect.rotationAngle)

                            if (radius < rotationCenterMark.radius) {
                                overPoint = true
                                overCross = false
                            } else {
                                overPoint = false
                                if(radius * Math.sin(deltaAngle*Math.PI/180) < 30*uiScale) {
                                    overCross = true
                                } else {
                                    overCross = false
                                }
                            }
                        }
                    }
                    onReleased: {
                        imageRect.readRotationPointX = Math.max(0, Math.min(1, imageRect.readRotationPointX))
                        imageRect.readRotationPointX = Math.max(0, Math.min(1, imageRect.readRotationPointX))
                        centering = false
                        rotating = false
                        preventStealing = false
                    }
                    onDoubleClicked: {
                        if (pressedButtons & Qt.RightButton) {
                            imageRect.rotationAngle = 0
                        }
                    }
                    onExited: {
                        if (!rotationDrag.pressed) {
                            overPoint = false
                            overCross = false
                        }
                    }
                    Connections {
                        target: bottomImage
                        function onWidthChanged() {
                            if (rotationDrag.pressed) {
                                console.log("updating oldx")
                                console.log(rotationDrag.oldX)
                                rotationDrag.oldX = rotationDrag.oldX*bottomImage.width/rotationDrag.oldImageWidth
                                console.log(rotationDrag.oldX)
                            }
                            rotationDrag.oldImageWidth = bottomImage.width
                        }
                        function onHeightChanged() {
                            if (rotationDrag.pressed) {
                                console.log("updating oldy")
                                console.log(rotationDrag.oldY)
                                rotationDrag.oldY = rotationDrag.oldY*bottomImage.height/rotationDrag.oldImageHeight
                                console.log(rotationDrag.oldY)
                            }
                            rotationDrag.oldImageHeight = bottomImage.height
                        }
                    }
                }

                // White balance picker stuff

                // We'll select a point relative to the image boundaries.
                // There won't be any stored data.
                // Note that these are relative to the already cropped image, so later on we need
                // to transform them according to the crop parameters.
                property real wbPointX
                property real wbPointY
                property real displayWbPointX: wbPointX*bottomImage.width
                property real displayWbPointY: wbPointY*bottomImage.height

                Rectangle {
                    id: verticalWbMark
                    width: 1
                    height: 2 * Math.max(bottomImage.height*bottomImage.scale, imageRect.height)
                    x: bottomImage.x + imageRect.displayWbPointX*bottomImage.scale - width/2
                    y: bottomImage.y + imageRect.displayWbPointY*bottomImage.scale - height/2
                    color: photoBox.backgroundColor == 2 ? "black" : photoBox.backgroundColor == 1 ? "gray" : "white"
                    visible: root.wbPicking && root.imageReady && imageRect.wbPointX >= 0 && imageRect.wbPointY >= 0
                }
                Rectangle {
                    id: horizontalWbMark
                    width: 2 * Math.max(bottomImage.height*bottomImage.scale, imageRect.width)
                    height: 1
                    x: bottomImage.x + imageRect.displayWbPointX*bottomImage.scale - width/2
                    y: bottomImage.y + imageRect.displayWbPointY*bottomImage.scale - height/2
                    color: photoBox.backgroundColor == 2 ? "black" : photoBox.backgroundColor == 1 ? "gray" : "white"
                    visible: root.wbPicking && root.imageReady && imageRect.wbPointX >= 0 && imageRect.wbPointY >= 0
                }
                Rectangle {
                    id: wbBox
                    width: 21 * bottomImage.scale
                    height: 21 * bottomImage.scale
                    x: bottomImage.x + imageRect.displayWbPointX*bottomImage.scale - width/2
                    y: bottomImage.y + imageRect.displayWbPointY*bottomImage.scale - height/2
                    color: "#00000000"
                    border.color: photoBox.backgroundColor == 2 ? "black" : photoBox.backgroundColor == 1 ? "gray" : "white"
                    visible: root.wbPicking && root.imageReady && imageRect.wbPointX >= 0 && imageRect.wbPointY >= 0
                }

                MouseArea {
                    id: wbDrag
                    acceptedButtons: Qt.LeftButton
                    width: bottomImage.width
                    height: bottomImage.height
                    x: bottomImage.x
                    y: bottomImage.y
                    transform: Scale {
                        origin.x: 0
                        origin.y: 0
                        xScale: bottomImage.scale
                        yScale: bottomImage.scale
                    }

                    enabled: root.wbPicking && root.imageReady
                    visible: root.wbPicking && root.imageReady
                    hoverEnabled: true
                    preventStealing: false

                    onPositionChanged: {
                        imageRect.wbPointX = mouse.x / bottomImage.width
                        imageRect.wbPointY = mouse.y / bottomImage.height
                    }
                    onPressed: {
                        imageRect.wbPointX = mouse.x / bottomImage.width
                        imageRect.wbPointY = mouse.y / bottomImage.height
                        root.wbPicking = false
                        filmProvider.customWB(imageRect.wbPointX, imageRect.wbPointY)
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
                    //For cropping and leveling, we don't want any surprise motions.
                    if (root.cropping || root.leveling) {
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

        Rectangle {
            id: savedIndicator
            anchors.centerIn: flicky
            width: savedText.contentWidth + 20 * uiScale
            height: 30 * uiScale
            color: "black"
            border.color: "gray"
            radius: 10 * uiScale
            Text {
                id: savedText
                anchors.centerIn: parent
                width: parent.width
                color: "white"
                text: qsTr("Image Saved")
                font.pixelSize: 20 * uiScale
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
            }
            state: "notVisible"
            states: [
                State {
                    name: "notVisible"
                    PropertyChanges {
                        target: savedIndicator
                        opacity: 0
                    }
                },
                State {
                    name: "isVisible"
                    PropertyChanges {
                        target: savedIndicator
                        opacity: 1
                    }
                }
            ]
            transitions: [
                Transition {
                    from: "isVisible"
                    to: "notVisible"
                    PropertyAnimation {
                        target: savedIndicator
                        properties: "opacity"
                        duration: 600
                    }
                },
                Transition {
                    from: "notVisible"
                    to: "isVisible"
                    PropertyAnimation {
                        target: savedIndicator
                        properties: "opacity"
                        duration: 0
                    }
                }
            ]
            Connections {
                target: editTools
                function onSaveStatusChanged() {
                    if (editTools.saveStatus == "") {
                        savedIndicator.state = "notVisible"
                    } else {
                        savedIndicator.state = "isVisible"
                        savedIndicatorTimer.start()
                    }
                }
            }
            Timer {
                id: savedIndicatorTimer
                interval: 1500
                onTriggered: {
                    savedIndicator.state = "notVisible"
                    editTools.saveStatus = ""
                }
            }
        }

        FilmProgressBar {
            id: progressBar
            visible: true
            value: filmProvider.progress
            Connections {
                target: filmProvider
                function onProgressChanged() { progressBar.value = filmProvider.progress }
            }
            uiScale: root.uiScale
        }
        Text {
            id: apertureText
            x: 202 * uiScale
            y: 1 * uiScale
            color: "white"
            text: "f/" + paramManager.aperture
            font.pixelSize: 12.0 * uiScale
            elide: Text.ElideRight
            visible: !root.cropping && !root.leveling && !parent.loadingError
        }
        Text {
            id: shutterText
            x: 202 * uiScale
            y: 15 * uiScale
            color: "white"
            text: paramManager.exposureTime + " s"
            font.pixelSize: 12.0 * uiScale
            elide: Text.ElideRight
            visible: !root.cropping && !root.leveling && !parent.loadingError
        }
        Text {
            id: focallengthText
            x: 270 * uiScale
            y: 1 * uiScale
            color: "white"
            text: paramManager.focalLength.toFixed(1) + "mm"
            font.pixelSize: 12.0 * uiScale
            elide: Text.ElideRight
            visible: !root.cropping && !root.leveling && !parent.loadingError
        }
        Text {
            id: isoText
            x: 270 * uiScale
            y: 15 * uiScale
            color: "white"
            text: "ISO " + paramManager.sensitivity
            font.pixelSize: 12.0 * uiScale
            elide: Text.ElideRight
            visible: !root.cropping && !root.leveling && !parent.loadingError
        }
        Text {
            id: filenameText
            x: 340 * uiScale
            y: 1 * uiScale
            width: lensfunBox.x - x
            color: "white"
            text: paramManager.filename
            font.pixelSize: 12.0 * uiScale
            elide: Text.ElideRight
            visible: !root.cropping && !parent.loadingError
            ToolTip {
                id: fileInfoTooltip
                anchors.fill: parent
                tooltipText: paramManager.fullFilenameQstr
                Component.onCompleted: {
                    fileInfoTooltip.tooltipWanted.connect(root.tooltipWanted)
                }
            }
        }
        Text {
            id: cameraText
            x: 340 * uiScale
            y: 15 * uiScale
            width: lensfunBox.x - x
            color: "white"
            text: paramManager.model
            font.pixelSize: 12.0 * uiScale
            elide: Text.ElideRight
            visible: !root.cropping && !parent.loadingError
        }
        Text {
            id: cropWidthText
            x: 202 * uiScale
            y: 1 * uiScale
            color: "white"
            text: qsTr("Width: ") + imageRect.displayWidth
            font.pixelSize: 12.0 * uiScale
            elide: Text.ElideRight
            visible: root.cropping && !parent.loadingError
        }
        Text {
            id: cropHeightText
            x: 202 * uiScale
            y: 15 * uiScale
            color: "white"
            text: qsTr("Height: ") + imageRect.displayHeight
            font.pixelSize: 12.0 * uiScale
            elide: Text.ElideRight
            visible: root.cropping && !parent.loadingError
        }
        Text {
            id: hOffsetText
            x: 300 * uiScale
            y: 1 * uiScale
            color: "white"
            text: qsTr("H offset: ") + imageRect.displayHoffset
            font.pixelSize: 12.0 * uiScale
            elide: Text.ElideRight
            visible: root.cropping && !parent.loadingError
        }
        Text {
            id: vOffsetText
            x: 300 * uiScale
            y: 15 * uiScale
            color: "white"
            text: qsTr("V offset: ") + imageRect.displayVoffset
            font.pixelSize: 12.0 * uiScale
            elide: Text.ElideRight
            visible: root.cropping && !parent.loadingError
        }
        Text {
            id: rotationText
            x: 202 * uiScale
            y: 1 * uiScale
            color: "white"
            text: qsTr("Rotation: ") + imageRect.rotationAngle.toFixed(2) + "°"
            font.pixelSize: 12.0 * uiScale
            elide: Text.ElideRight
            visible: root.leveling && !parent.loadingError
        }

        Rectangle {
            id: errorBox
            x: 200 * uiScale
            y: 0 * uiScale
            width: parent.width - ((200 + 14*4 + 2) * uiScale)
            height: Math.ceil(30 * uiScale)
            color: "black"
            visible: parent.loadingError

            Image {
                id: errorIcon
                width: 25 * uiScale
                height: 25 * uiScale
                x: 2 * uiScale
                y: 4 * uiScale
                source: "qrc:///icons/errortriangle.png"
                antialiasing: true
            }

            Text {
                id: errorTextInfo
                x: 28 * uiScale
                y: 0 * uiScale
                width: parent.width - x
                height: parent.height
                text: qsTr("Error: ") + photoBox.errorText + qsTr(" is not accessible.")
                color: "white"
                font.pixelSize: 12.0 * uiScale
                elide: Text.ElideMiddle
                verticalAlignment: Text.AlignVCenter
            }
        }

        //right-aligned stuff

        Rectangle {
            id: lensfunBox
            x: leftButtonSpacer.x - (470 - 180) * uiScale //not width because we don't want it to move when it resizes
            y: 0 * uiScale
            z: active ? 1 : 0
            //resize when active to make room for german translation of buttons at bottom
            //180 is the width of the buttons, 2 is the padding to make the rightmost button stationary
            width: active ? (470 + 2) * uiScale : (470 - 180) * uiScale
            height: active ? 400 * uiScale : 30 * uiScale
            radius: 5 * uiScale
            visible: !photoBox.loadingError
            property bool active: false
            color: active ? Colors.darkGray : "black"

            ToolButton {
                id: lensFunMenuButton
                x: 0 * uiScale
                y: 0 * uiScale
                tooltipText: qsTr("Select the lens that was used in order to use lens corrections.\n\nType the lens name in the box to the right. Normally it will only search lenses for the camera's mount, but if the first character is a backslash (\"")+"\\"+qsTr("\") then it will search lenses from all mounts.\n\nDouble-click a lens to select it.")
                Image {
                    id: lensFunMenuButtonImage
                    width: 14 * uiScale
                    height: 14 * uiScale
                    anchors.centerIn: parent
                    source: "qrc:///icons/uparrow.png"
                    antialiasing: true
                    rotation: lensfunBox.active ? 0 : 180
                }
                onTriggered: {
                    lensfunBox.active = !lensfunBox.active
                }
                Component.onCompleted: {
                    lensFunMenuButton.tooltipWanted.connect(root.tooltipWanted)
                }
                uiScale: root.uiScale
            }

            Rectangle {
                id: textEntryRect
                color: "black"
                x: 32 * uiScale
                y: 3 * uiScale
                width: parent.width - 36*uiScale
                height: 24 * uiScale
                property string selectedLens: paramManager.lensfunName
                onSelectedLensChanged: {
                    paramManager.lensfunName = selectedLens
                    paramManager.writeback()
                }
                Connections {
                    target: paramManager
                    function onLensfunNameChanged() {
                        textEntryRect.selectedLens = paramManager.lensfunName
                    }
                }

                Text {
                    id: selectedLensText
                    x: 3 * uiScale
                    y: 3 * uiScale
                    width: parent.width-2*x
                    height: parent.height-2*y
                    color: "white"
                    font.pixelSize: 12.0 * uiScale
                    clip: true
                    visible: !lensfunBox.active
                    text: (parent.selectedLens == "") ? qsTr("No lens selected") : (parent.selectedLens.charAt(0)=="\\") ? parent.selectedLens.slice(1) : parent.selectedLens
                }

                MouseArea {
                    id: textEntryMouseArea
                    anchors.fill: parent
                    acceptedButtons: Qt.LeftButton
                    enabled: !lensfunBox.active
                    onClicked: {
                        lensfunBox.active = true
                        lensFilterBox.forceActiveFocus()
                    }
                }

                TextInput {
                    id: lensFilterBox
                    x: 3 * uiScale
                    y: 3 * uiScale
                    width: parent.width - 2*x - 30 * uiScale
                    height: parent.height - 2*y
                    color: "white"
                    selectByMouse: true
                    cursorVisible: focus
                    font.pixelSize: 12.0 * uiScale
                    clip: true
                    visible: lensfunBox.active
                    onTextChanged: {
                        lensModel.update(paramManager.model, lensFilterBox.text)
                    }
                    Connections {
                        target: paramManager
                        function onExifLensNameChanged() {
                            lensFilterBox.text = paramManager.exifLensName
                        }
                    }
                }
            }
            ToolButton {
                id: lensFunResetButton
                x: parent.width - 30*uiScale
                y: 0 * uiScale
                visible: lensfunBox.active
                tooltipText: qsTr("Reset selected lens back to default and sets search box back to EXIF-derived lens name.")
                Image {
                    width: 14 * uiScale
                    height: 14 * uiScale
                    anchors.centerIn: parent
                    source: "qrc:///icons/refresh.png"
                    antialiasing: true
                }
                onTriggered: {
                    lensFilterBox.text = paramManager.exifLensName
                    textEntryRect.selectedLens = paramManager.defLensfunName
                    paramManager.resetLensfunName()
                    paramManager.writeback()
                }
                Component.onCompleted: {
                    lensFunResetButton.tooltipWanted.connect(root.tooltipWanted)
                }
                uiScale: root.uiScale
            }
            ListView {
                id: lensListBox
                x: 3 * uiScale
                y: 33 * uiScale
                width: parent.width - 6*uiScale
                height: parent.height - 66*uiScale
                visible: lensfunBox.active
                orientation: ListView.Vertical
                spacing: 3 * uiScale
                clip: true

                boundsBehavior: Flickable.StopAtBounds
                flickDeceleration: 6000 * uiScale
                maximumFlickVelocity: 10000 * Math.sqrt(uiScale)

                onMovingChanged: { //reset params after mouse scrolling
                    if (!moving) {
                        flickDeceleration = 6000 * uiScale
                        maximumFlickVelocity = 10000 * Math.sqrt(uiScale)
                    }
                }

                delegate: Rectangle {
                    id: lensListDelegate
                    width: lensListBox.width - 6 * uiScale
                    height: 40 * uiScale
                    radius: 5 * uiScale
                    color: (lensName === textEntryRect.selectedLens) ? Colors.darkGrayH : Colors.darkGrayL
                    property string lensMake: make
                    property string fullLensName: model
                    property string lensName: fullLensName.charAt(0) === "\\" ? fullLensName.slice(1) : fullLensName
                    property int matchScore: score
                    Text {
                        id: lensNameText
                        x: 5 * uiScale
                        y: 5 * uiScale
                        width: parent.width - 10*uiScale
                        height: 20 * uiScale
                        color: "white"
                        elide: Text.ElideMiddle
                        text: (parent.lensMake.length===0) ? parent.lensName : (parent.lensName.startsWith(parent.lensMake) ? parent.lensName : (parent.lensMake + " " + parent.lensName))
                    }
                    Text {
                        id: lensScoreText
                        x: 5 * uiScale
                        y: 20 * uiScale
                        width: parent.width - 10*uiScale
                        height: 20 * uiScale
                        color: "white"
                        text: qsTr("Search fit score: ") + parent.matchScore
                    }
                    MouseArea {
                        id: lensSelectMouseArea
                        anchors.fill: parent
                        acceptedButtons: Qt.LeftButton
                        onClicked: {
                            textEntryRect.selectedLens = parent.fullLensName
                        }
                        onDoubleClicked: {
                            textEntryRect.selectedLens = parent.fullLensName
                            lensfunBox.active = false
                        }
                    }
                }
                Item {
                    id: scrollbarHolderLensListBox
                    x: parent.width - 10*uiScale
                    y: 0
                    width: 10*uiScale
                    height: parent.height

                    Rectangle {
                        id: scrollbarBackgroundLensListBox
                        color: Colors.darkGray
                        opacity: 0

                        x: parent.width-width - 1*uiScale
                        width: 3 * uiScale

                        y: 0
                        height: parent.height

                        transitions: Transition {
                            NumberAnimation {
                                property: "width"
                                duration: 200
                            }
                            NumberAnimation {
                                property: "opacity"
                                duration: 200
                            }
                        }
                        states: State {
                            name: "hovered"
                            when: scrollbarMouseAreaLensListBox.containsMouse || scrollbarMouseAreaLensListBox.pressed
                            PropertyChanges {
                                target: scrollbarBackgroundLensListBox
                                width: 8 * uiScale
                                opacity: 0.5
                            }
                        }
                    }

                    Rectangle {
                        id: scrollbarLensListBox
                        color: scrollbarMouseAreaLensListBox.pressed ? Colors.medOrange : scrollbarMouseAreaLensListBox.containsMouse ? Colors.weakOrange : Colors.middleGray
                        radius: 1.5*uiScale

                        x: parent.width-width - 1 * uiScale
                        width: 3 * uiScale

                        y: 1 * uiScale + (0.99*lensListBox.visibleArea.yPosition) * (parent.height - 2*uiScale)
                        height: (0.99*lensListBox.visibleArea.heightRatio + 0.01) * (parent.height - 2*uiScale)

                        transitions: Transition {
                            NumberAnimation {
                                property: "width"
                                duration: 200
                            }
                        }
                        states: State {
                            name: "hovered"
                            when: scrollbarMouseAreaLensListBox.containsMouse || scrollbarMouseAreaLensListBox.pressed
                            PropertyChanges {
                                target: scrollbarLensListBox
                                width: 8 * uiScale
                            }
                        }
                    }
                    MouseArea {
                        id: scrollbarMouseAreaLensListBox
                        anchors.fill: parent
                        hoverEnabled: true
                        acceptedButtons: Qt.LeftButton
                        onWheel: {
                            //See the Queue.qml file for the math behind this.

                            //We have to duplicate the wheelstealer one because this has higher priority for some reason.
                            //Set the scroll deceleration and max speed higher for wheel scrolling.
                            //It should be reset when the view stops moving.
                            //For now, this is 10x higher than standard.
                            var deceleration = 6000 * 10
                            lensListBox.flickDeceleration = deceleration * uiScale
                            lensListBox.maximumFlickVelocity = 10000 * Math.sqrt(uiScale*10)

                            var velocity = lensListBox.verticalVelocity/uiScale
                            var newVelocity = velocity

                            var distance = 100
                            if (wheel.angleDelta.y > 0 && !lensListBox.atXBeginning && !root.dragging) {
                                //Leftward; up on the scroll wheel.
                                newVelocity = uiScale*(velocity <= 0 ? Math.sqrt((velocity*velocity/(4*deceleration) + distance*wheel.angleDelta.y/(120))*4*deceleration) : 0)
                                newVelocity = Math.min(newVelocity, lensListBox.maximumFlickVelocity)
                                lensListBox.flick(0,1)
                                lensListBox.flick(0, newVelocity)
                            } else if (wheel.angleDelta.y < 0 && !lensListBox.atXEnd && !root.dragging) {
                                //Rightward; down on the scroll wheel.
                                newVelocity = uiScale*(velocity >= 0 ? Math.sqrt((velocity*velocity/(4*deceleration) + distance*wheel.angleDelta.y/(-120))*4*deceleration) : 0)
                                newVelocity = -Math.min(newVelocity, lensListBox.maximumFlickVelocity)
                                lensListBox.flick(0,-1)
                                lensListBox.flick(0, newVelocity)
                            }
                        }

                        property bool overDragThresh: false
                        property real pressY
                        property real viewY
                        onPositionChanged: {
                            if (pressed) {
                                var deltaY = mouse.y - pressY
                                var scrollHeight = scrollbarMouseAreaLensListBox.height - scrollbarLensListBox.height - 2*uiScale
                                var relativeDelta = deltaY / scrollHeight
                                var scrollMargin = lensListBox.contentHeight - lensListBox.height
                                lensListBox.contentY = Math.max(0, Math.min(scrollMargin, viewY + relativeDelta * scrollMargin))
                            }
                        }

                        onPressed: {
                            preventStealing = true
                            lensListBox.cancelFlick()
                            pressY = mouse.y
                            viewY = lensListBox.contentY
                        }
                        onReleased: {
                            preventStealing = false
                        }
                    }
                }
                MouseArea {
                    id: wheelStealer
                    //Custom scrolling implementation because the default flickable one sucks.
                    anchors.fill: lensListBox
                    acceptedButtons: Qt.NoButton
                    onWheel: {
                        //See the Queue.qml file for the math behind this.

                        //Set the scroll deceleration and max speed higher for wheel scrolling.
                        //It should be reset when the view stops moving.
                        //For now, this is 10x higher than standard.
                        var deceleration = 6000 * 2
                        lensListBox.flickDeceleration = deceleration * uiScale
                        lensListBox.maximumFlickVelocity = 10000 * Math.sqrt(uiScale*2)

                        var velocity = lensListBox.verticalVelocity/uiScale
                        var newVelocity = velocity

                        var distance = 30 //the tool list is relatively short so it needs less scrolling
                        if (wheel.angleDelta.y > 0 && !lensListBox.atYBeginning) {
                            //up on the scroll wheel.
                            newVelocity = uiScale*(velocity <= 0 ? Math.sqrt((velocity*velocity/(4*deceleration) + distance*wheel.angleDelta.y/(120))*4*deceleration) : 0)
                            newVelocity = Math.min(newVelocity, lensListBox.maximumFlickVelocity)
                            lensListBox.flick(0,1)
                            lensListBox.flick(0, newVelocity)
                        } else if (wheel.angleDelta.y < 0 && !lensListBox.atYEnd) {
                            //down on the scroll wheel.
                            newVelocity = uiScale*(velocity >= 0 ? Math.sqrt((velocity*velocity/(4*deceleration) + distance*wheel.angleDelta.y/(-120))*4*deceleration) : 0)
                            newVelocity = -Math.min(newVelocity, lensListBox.maximumFlickVelocity)
                            lensListBox.flick(0,-1)
                            lensListBox.flick(0, newVelocity)
                        }
                    }
                }

                Component.onCompleted: {
                    lensListBox.model = lensModel
                }
            }
            ToolButton {
                id: savePreferredLens
                x: 0 * uiScale
                y: parent.height - 30 * uiScale;
                width: parent.width/2
                visible: lensfunBox.active
                text: qsTr("Remember preferred lens")
                tooltipText: qsTr("Use the selected lens as default for all future photos taken with the same camera and lens combination.\n\nThis also remembers the currently selected lens corrections.")
                onTriggered: {
                    //store exif camera, exif lens name, and lensfun lens name in database, plus preferred corrections
                    paramManager.setLensPreferences()
                }
                Component.onCompleted: {
                    savePreferredLens.tooltipWanted.connect(root.tooltipWanted)
                }
                uiScale: root.uiScale
            }
            ToolButton {
                id: forgetPreferredLens
                x: parent.width/2
                y: parent.height - 30 * uiScale;
                width: parent.width/2
                visible: lensfunBox.active
                text: qsTr("Forget preferred lens")
                tooltipText: qsTr("Clear the default lens for photos taken with the same camera and lens combination.")
                onTriggered: {
                    //clear exif camera, exif lens name, and lensfun lens name in database
                    paramManager.eraseLensPreferences()
                }
                Component.onCompleted: {
                    forgetPreferredLens.tooltipWanted.connect(root.tooltipWanted)
                }
                uiScale: root.uiScale
            }
        }

        Item {
            id: leftButtonSpacer
            anchors.right: backgroundBrightness.left
            y: 0 * uiScale
            width: 2 * uiScale
            height: 2 * uiScale
        }
        ToolButton {
            id: backgroundBrightness
            anchors.right: wbButton.left
            y: 0 * uiScale
            tooltipText: qsTr("Change the editor's background brightness between black, gray, and white.\n\nShortcut: B")
            Image {
                width: 14 * uiScale
                height: 14 * uiScale
                anchors.centerIn: parent
                source: "qrc:///icons/brightness.png"
                antialiasing: true
            }
            onTriggered: {
                if (photoBox.backgroundColor == 0) {
                    photoBox.backgroundColor = 1
                } else if (photoBox.backgroundColor == 1) {
                    photoBox.backgroundColor = 2
                } else {
                    photoBox.backgroundColor = 0
                }
            }

            Shortcut {
                sequence: "b"
                onActivated: {
                    if (root.onEditTab) {
                        if (photoBox.backgroundColor == 0) {
                            photoBox.backgroundColor = 1
                        } else if (photoBox.backgroundColor == 1) {
                            photoBox.backgroundColor = 2
                        } else {
                            photoBox.backgroundColor = 0
                        }
                    }
                }
            }

            Component.onCompleted: {
                backgroundBrightness.tooltipWanted.connect(root.tooltipWanted)
            }
            uiScale: root.uiScale
        }

        ToolButton {
            id: wbButton
            anchors.right: crop.left
            y: 0 * uiScale
            notDisabled: root.imageReady && !root.cropping && !root.leveling
            tooltipText: root.wbPicking ? qsTr("Cancel setting custom white balance. Shortcut: W") : qsTr("Set a custom white balance based on where you click in the image.\n\nThe white balance will be remembered and can be applied to other images taken by the same camera model.\n\nShortcut: W")
            Image {
                width: 14 * uiScale
                height: 14 * uiScale
                anchors.centerIn: parent
                source: root.wbPicking ? "qrc:///icons/whitebalanceactive.png" : "qrc:///icons/whitebalance.png"
                antialiasing: true
                opacity: wbButton.notDisabled ? 1 : 0.5
            }
            onTriggered: {
                if (!root.wbPicking) {
                    imageRect.wbPointX = -1
                    imageRect.wbPointY = -1
                    root.wbPicking = true
                } else {
                    root.wbPicking = false
                }
            }
            Shortcut {
                sequence: "w"
                enabled: wbButton.notDisabled && root.onEditTab
                onActivated: {
                    if (!root.wbPicking) {
                        imageRect.wbPointX = -1
                        imageRect.wbPointY = -1
                        root.wbPicking = true
                    } else {
                        root.wbPicking = false
                    }
                }
            }

            Component.onCompleted: {
                wbButton.tooltipWanted.connect(root.tooltipWanted)
            }
            uiScale: root.uiScale
        }

        ToolButton {
            id: crop
            anchors.right: rotateLeft.left
            y: 0 * uiScale
            notDisabled: root.imageReady && !root.wbPicking && !root.leveling
            tooltipText: (root.cropping ? qsTr("Click this to save your crop."): qsTr("Click this to begin cropping.")) + "\n\n" + qsTr("Hold Ctrl when dragging a corner to lock aspect ratio. Hold Ctrl while dragging an edge or the remaining image to move the crop without changing its size.\n\nHold Shift while dragging a corner to snap the crop to the nearest common aspect ratio. Hold Shift while moving the crop to snap it to horizontal and or vertical center.\n\nShortcut: C")
            Image {
                width: 14 * uiScale
                height: 14 * uiScale
                anchors.centerIn: parent
                source: root.cropping ? "qrc:///icons/cropactive.png" : "qrc:///icons/crop.png"
                antialiasing: true
                opacity: crop.notDisabled ? 1 : 0.5
            }
            onTriggered: {
                if (!root.cropping) {
                    filmProvider.disableThumbnailWrite()
                    root.requestingCropping = true
                } else {
                    filmProvider.enableThumbnailWrite()
                    root.cancelCropping = false //we're saving the crop, not canceling it
                    root.requestingCropping = false
                }
            }

            Shortcut {
                sequence: "c"
                enabled: crop.notDisabled && root.onEditTab
                onActivated: {
                    if (!root.cropping) {
                        filmProvider.disableThumbnailWrite()
                        root.requestingCropping = true
                    } else {
                        filmProvider.enableThumbnailWrite()
                        root.cancelCropping = false //we're saving the crop, not canceling it
                        root.requestingCropping = false
                    }
                }
            }

            Component.onCompleted: {
                crop.tooltipWanted.connect(root.tooltipWanted)
            }
            uiScale: root.uiScale
        }

        ToolButton {
            id: rotateLeft
            anchors.right: level.left
            y: 0 * uiScale
            notDisabled: root.imageReady && !root.wbPicking && !root.cropping && !root.leveling
            tooltipText: qsTr("Rotate image 90 degrees left.")
            Image {
                width: 14 * uiScale
                height: 14 * uiScale
                anchors.centerIn: parent
                source: "qrc:///icons/rotateleft.png"
                antialiasing: true
                opacity: rotateLeft.notDisabled ? 1 : 0.5
            }
            onTriggered: {
                paramManager.rotateLeft()
            }
            Component.onCompleted: {
                rotateLeft.tooltipWanted.connect(root.tooltipWanted)
            }
            uiScale: root.uiScale
        }

        ToolButton {
            id: level
            anchors.right: rotateRight.left
            y: 0 * uiScale
            notDisabled: root.previewReady && !root.wbPicking && !root.cropping
            tooltipText: (root.leveling ? qsTr("Click this to apply the rotation.") : qsTr("Click this to begin leveling the image.")) + "\n\n" + qsTr("Click to place the rotation guide on the image, then drag the guide lines to align them with whatever you want to be vertical or horizontal. You can reposition the rotation guide by dragging where the guide lines meet.\n\nReset the rotation to zero by pressing \"Shift+L\" or double right clicking.\n\nShortcut: L")
            Image {
                width: 14 * uiScale
                height: 14 * uiScale
                anchors.centerIn: parent
                source: root.requestingLeveling ? "qrc:///icons/levelactive.png" : "qrc:///icons/level.png"
                antialiasing: true
                opacity: level.notDisabled ? 1 : 0.5
            }
            onTriggered: {
                if (!root.leveling) {
                    filmProvider.disableThumbnailWrite()
                    root.requestingLeveling = true
                } else {
                    filmProvider.enableThumbnailWrite()
                    root.cancelLeveling = false //we're saving the leveling, not canceling it
                    root.requestingLeveling = false
                }
            }

            Shortcut {
                sequence: "l"
                enabled: level.notDisabled && root.onEditTab
                onActivated: {
                    if (!root.leveling) {
                        filmProvider.disableThumbnailWrite()
                        root.requestingLeveling = true
                    } else {
                        filmProvider.enableThumbnailWrite()
                        root.cancelLeveling = false //we're saving the leveling, not canceling it
                        root.requestingLeveling = false
                    }
                }
            }
            Shortcut {
                sequence: "Shift+l"
                enabled: root.leveling && root.previewReady && root.onEditTab
                onActivated: {
                    imageRect.rotationAngle = 0
                }
            }

            Component.onCompleted: {
                level.tooltipWanted.connect(root.tooltipWanted)
            }
            uiScale: root.uiScale
        }

        ToolButton {
            id: rotateRight
            anchors.right: rightSpacer.left
            y: 0 * uiScale
            notDisabled: root.imageReady && !root.wbPicking && !root.cropping && !root.leveling
            tooltipText: qsTr("Rotate image 90 degrees right.")
            Image {
                width: 14 * uiScale
                height: 14 * uiScale
                anchors.centerIn: parent
                source: "qrc:///icons/rotateleft.png"
                mirror: true
                antialiasing: true
                opacity: rotateRight.notDisabled ? 1 : 0.5
            }
            onTriggered: {
                paramManager.rotateRight()
            }
            Component.onCompleted: {
                rotateRight.tooltipWanted.connect(root.tooltipWanted)
            }
            uiScale: root.uiScale
        }
        Item {
            id: rightSpacer
            anchors.right: parent.right
            y: 0 * uiScale
            width: 2 * uiScale
            height: 2 * uiScale
        }

        Connections {
            target: paramManager
            function onFileError() {
                photoBox.errorText = paramManager.fullFilenameQstr
                photoBox.loadingError = true
            }
            function onFilenameChanged() {
                photoBox.loadingError = false
                photoBox.errorText = ""
            }
        }
    }
    EditTools {
        id: editTools
        uiScale: root.uiScale
        imageReady: root.imageReady
        previewReady: root.previewReady
        cropping: root.requestingCropping || root.cropping
        leveling: root.requestingLeveling || root.leveling
        onEditTab: root.onEditTab
        Component.onCompleted: {
            editTools.tooltipWanted.connect(root.tooltipWanted)
        }
    }
}
