import QtQuick 2.3
import QtQuick.Controls 1.2
import QtQuick.Layouts 1.1
import "gui_components"

SplitView {
    id: root
    anchors.fill: parent
    orientation: Qt.Horizontal
    property real uiScale: 1
    property bool imageReady: false

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
            contentWidth:  Math.max(bottomImage.width *bottomImage.scale, this.width);
            contentHeight: Math.max(bottomImage.height*bottomImage.scale, this.height);
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
                width: Math.floor(Math.max(bottomImage.width*bottomImage.scale,parent.width));
                height: Math.floor(Math.max(bottomImage.height*bottomImage.scale,parent.height));
                transformOrigin: Item.TopLeft
                color: photoBox.backgroundColor == 2 ? "white" : photoBox.backgroundColor == 1 ? "gray" : "black"
                Image {
                    x: Math.floor(parent.width/2) - Math.floor(width / 2)
                    y: Math.floor(parent.height/2) - Math.floor(height / 2)
                    id: topImage
                    fillMode: Image.PreserveAspectFit
                    asynchronous: true
                    mipmap: settings.getMipmapView()
                    property int index: 0
                    property string indexString: "000000"
                    //===================================================================IMPLEMENT THESE=======================================================================
                    //We need to
                    //property real customScale: bottomImage.customScale
                    //transform: Scale {xScale: customScale; yScale: customScale}
                    scale: bottomImage.scale

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
                                console.log("hidden image ready")
                                topImage.state = "i"
                                topImage.source = hiddenImage.source
                                root.imageReady = true
                            }
                            else {
                                root.imageReady = false
                            }
                        }
                    }

                    property string state: "p"
                    property string rootDir: organizeModel.thumbDir()
                    //This connection finds out when the parameters are done changing, and then updates the url to fetch the latest image.
                    Connections {
                        target: paramManager//root
                        onUpdateImage: {
                            console.log("update image")
                            if (topImage.state == "i") {//only if we're done with the thumbnail
                                var num = (topImage.index + 1) % 1000000//1 in a million
                                topImage.index = num;
                                var s = num+"";
                                var size = 6 //6 digit number
                                while (s.length < size) {s = "0" + s}
                                topImage.indexString = s
                                //topImage.source = "image://filmy/" + topImage.indexString
                                hiddenImage.source = "image://filmy/" + topImage.indexString
                            }
                            if (topImage.state == "p") {//if we're planning on doing the thumbnail
                                var thumbPath = topImage.rootDir + '/' + paramManager.imageIndex.slice(0,4) + '/' + paramManager.imageIndex + '.jpg'
                                console.log("thumb path: " + thumbPath)
                                topImage.source = thumbPath
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
                                root.imageURL(topImage.source)
                            }
                            if (topImage.state == "p") {//if the thumbnail or full size preview is loaded
                                //now we say that the state is loading the actual image, so as to not load a new preview.
                                topImage.state = "i"
                                console.log("TopImage state: " + topImage.state)

                                //begin loading the main image
                                var num = (topImage.index + 1) % 1000000//1 in a million
                                topImage.index = num;
                                var s = num+"";
                                var size = 6 //6 digit number
                                while (s.length < size) {s = "0" + s}
                                topImage.indexString = s
                                //topImage.source = "image://filmy/" + topImage.indexString
                                hiddenImage.source = "image://filmy/" + topImage.indexString
                            }
                        }
                    }
                }
                Image {
                    x: Math.floor(parent.width/2) - Math.floor(width / 2)
                    y: Math.floor(parent.height/2) - Math.floor(height / 2)
                    id: bottomImage
                    fillMode: Image.PreserveAspectFit
                    asynchronous: true
                    mipmap: settings.getMipmapView()
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
                            flicky.contentX = 0
                            flicky.contentY = 0
                            //flicky.returnToBounds()
                            flicky.fit = true
                        }
                        else {//Currently, zooming in works perfectly from fit.
                            var zoomFactor = 1/bottomImage.scale

                            var oldContentX = flicky.contentX
                            var oldContentY = flicky.contentY

                            var oldMouseX = mouse.x - Math.max(0, 0.5*(flicky.width  -  bottomImage.width*bottomImage.scale))
                            var oldMouseY = mouse.y - Math.max(0, 0.5*(flicky.height - bottomImage.height*bottomImage.scale))

                            bottomImage.scale = 1

                            //for the following, the last bottomImage.scale is now 1, so we just leave it off.
                            flicky.contentX = oldMouseX*zoomFactor - mouse.x + oldContentX + Math.max(0, 0.5*(flicky.width  - bottomImage.width))
                            flicky.contentY = oldMouseY*zoomFactor - mouse.y + oldContentY + Math.max(0, 0.5*(flicky.height - bottomImage.height))

                            flicky.returnToBounds()
                            if (bottomImage.scale == flicky.fitScale) {flicky.fit = true}
                            else {flicky.fit = false}
                        }
                    }
                }

                //From here are the crop markers.
                //There are four parameters that get stored.
                // crop height as % of image height
                property real cropheight: 0.5312353
                // width / height (aspect ratio of the crop)
                property real cropaspect: 1.5
                // voffset as % of image height, center from center
                property real cropVoffset: 0.0
                // hoffset as % of image width, center from center
                property real cropHoffset: -0.3
                Rectangle {
                    id: cropmarker
                    color: 'green'
                    property real tempHeight: bottomImage.height * imageRect.cropheight
                    width: Math.min(tempHeight * imageRect.cropaspect, bottomImage.width)
                    height: Math.min(tempHeight, width / imageRect.cropaspect)
                    //Hoffsets need work.
                    property real maxHoffset: (1-(width /bottomImage.width ))/2
                    property real maxVoffset: (1-(height/bottomImage.height))/2
                    property real hoffset: Math.max(Math.min(imageRect.cropHoffset, maxHoffset), -maxHoffset)
                    property real voffset: Math.max(Math.min(imageRect.cropVoffset, maxVoffset), -maxVoffset)
                    x: Math.floor(parent.width/2)  - Math.floor(width/2)  + Math.floor(hoffset*bottomImage.width*scale)
                    y: Math.floor(parent.height/2) - Math.floor(height/2) + Math.floor(voffset*bottomImage.height*scale)
                    scale: bottomImage.scale
                }

                //Test readouts of the properties for writing back to database.
                property real readheight: cropmarker.height / bottomImage.height
                property real readwidth: cropmarker.width / cropmarker.width
                property real readHoffset: cropmarker.hoffset
                property real readVoffset: cropmarker.voffset
            }
        }
        MouseArea {
            id: wheelCapture
            anchors.fill: flicky
            acceptedButtons: Qt.RightButton
            onWheel: {
                var oldMouseX = wheel.x + flicky.contentX - Math.max(0, 0.5*(flicky.width-bottomImage.width*bottomImage.scale))
                var oldMouseY = wheel.y + flicky.contentY - Math.max(0, 0.5*(flicky.height-bottomImage.height*bottomImage.scale))
                //1.2 is the zoom factor for a normal wheel click, and 120 units is the 'angle' of a normal wheel click.
                var zoomFactor = Math.pow(1.2,Math.abs(wheel.angleDelta.y)/120)
                if (wheel.angleDelta.y > 0) {
                    bottomImage.scale *= zoomFactor;
                    flicky.contentX = oldMouseX*zoomFactor - wheel.x + Math.max(0, 0.5*(flicky.width-bottomImage.width*bottomImage.scale))
                    flicky.contentY = oldMouseY*zoomFactor - wheel.y + Math.max(0, 0.5*(flicky.height-bottomImage.height*bottomImage.scale))
                }
                else {
                    bottomImage.scale /= zoomFactor;
                    flicky.contentX = oldMouseX/zoomFactor - wheel.x + Math.max(0, 0.5*(flicky.width  -  bottomImage.width*bottomImage.scale))
                    flicky.contentY = oldMouseY/zoomFactor - wheel.y + Math.max(0, 0.5*(flicky.height - bottomImage.height*bottomImage.scale))
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
            text: qsTr("Crop")//Change to "Adjust crop" when a crop exists; change to "Accept crop" when cropping in progress
            tooltipText: qsTr("Click this to begin cropping.")//change to "Hold shift to snap to common aspect ratios" when cropping in progress
            onTriggered: {
                cropmarker.visible = !cropmarker.visible
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
            //text: " f/" + paramManager.aperture
            text: imageRect.readheight
            font.pixelSize: 12.0 * uiScale
            elide: Text.ElideRight
        }
        Text {
            id: shutterText
            x: 200 * uiScale
            y: 15 * uiScale
            color: "white"
            //text: " " + paramManager.exposureTime + " s"
            text: imageRect.readwidth
            font.pixelSize: 12.0 * uiScale
            elide: Text.ElideRight
        }
        Text {
            id: filenameText
            //x: 300 * uiScale
            x: 350 * uiScale
            y: 0 * uiScale
            color: "white"
            //text: paramManager.filename
            text: imageRect.readHoffset
            font.pixelSize: 12.0 * uiScale
            elide: Text.ElideRight
        }
        Text {
            id: isoText
            //x: 300 * uiScale
            x: 350 * uiScale
            y: 15 * uiScale
            color: "white"
            //text: "ISO " + paramManager.sensitivity
            text: imageRect.readVoffset
            font.pixelSize: 12.0 * uiScale
            elide: Text.ElideRight
        }
    }
    EditTools {
        id: editTools
        uiScale: root.uiScale
        imageReady: root.imageReady
        Component.onCompleted: {
            editTools.tooltipWanted.connect(root.tooltipWanted)
        }
    }
}
