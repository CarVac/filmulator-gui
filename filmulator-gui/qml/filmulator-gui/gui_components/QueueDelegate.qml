import QtQuick 2.12
import "../colors.js" as Colors

Item {
    id: root
    property real dim
    property real widthScale: 1
    width: dim * widthScale
    height: dim


    property string rootDir

    property string selectedID
    property string searchID
    property int queueIndex
    property bool processed
    property bool exported
    property bool markedForOutput
    property int rating
    property bool rightClicked: false
    property bool held

    property string __thumbPath: (Qt.platform.os == "windows" ? 'file:///' : 'file://') + rootDir + '/' + searchID.slice(0,4) + '/' + searchID + '.jpg'

    property bool __current: searchID===selectedID
    property bool __waitingForThumb: false

    property string freshURL
    property string validFreshURL: ""

    onFreshURLChanged: {
        if (__current) {
            validFreshURL = freshURL
        }
    }

    on__CurrentChanged: {
        if (!__current) {//no longer the selected image
            if (!__waitingForThumb) {
                //make sure we're not waiting for the thumb lest it flicker
                validFreshURL = ""
            }
        }
    }

    signal openThisImage( string filePath, string sensitivity, string exposureTime, real aperture, real focalLength, real initialDeveloperConcentration, real reservoirThickness, real activeLayerThickness, real crystalsPerPixel, real initialCrystalRadius, real initialSilverSaltDensity, real developerConsumptionConst, real crystalGrowthConst, real silverSaltConsumptionConst, real totalDevelopmentTime, int agitateCount, int developmentResolution, real filmArea, real sigmaConst, real layerMixConst, real layerTimeDivisor, int rolloffBoundary, real exposureComp, real whitepoint, real blackpoint, real shadowsX, real shadowsY, real highlightsX, real highlightsY, int highlightRecovery, bool caEnabled, real temperature, real tint, real vibrance, real saturation, int orientation )
    signal refresh();

    Rectangle {
        id: currentImageRect
        width: root.width
        height: root.height*0.03125
        color: held ? Colors.brightOrange : (rightClicked ? Colors.whiteOrange : (__current ? Colors.medOrange : "#00000000"))
    }

    Loader {
        id: loadThumb
        asynchronous: true
    }

    Component {
        id: thumbImage
        Item {
            x: root.width * 0.03125
            y: root.height * 0.03125
            width: root.width * 0.9375
            height: root.height * 0.9375
            Image {
                id: thumb
                anchors.fill: parent
                fillMode: Image.PreserveAspectFit
                source: root.__thumbPath
                //sourceSize.width: 600
                //sourceSize.height: 600
                cache: false
                Connections {
                    target: filmProvider
                    function onThumbnailDone() {
                        if (__waitingForThumb) {
                            //console.log('thumb received')
                            //thumb.cache = false
                            thumb.source = ""
                            //thumb.cache = true
                            thumb.source = __thumbPath
                            __waitingForThumb = false
                            if (!__current) {
                                validFreshURL = ""
                            }
                        }
                    }
                }
            }
            Image {
                id: freshThumb
                anchors.fill: parent
                fillMode: Image.PreserveAspectFit
                visible: (__current || __waitingForThumb)
                source: validFreshURL
                onSourceChanged: {
                    if (__current) {
                        //console.log('thumb source changed and current')
                        var thumbSource = __thumbPath
                        //filmProvider.writeThumbnail(thumbSource.slice(0, -4))
                        filmProvider.writeThumbnail(searchID)
                        __waitingForThumb = true
                    }
                }
            }
        }
    }

    Rectangle {
        id: processedSavedRect
        width: root.width
        height: root.height*0.03125
        y: root.height*0.96875
        color: exported ? "green" : (processed ? Colors.medOrange : "#00000000")
    }
    //Instead of another color to indicate output queueing,
    //just place another green rectangle over the center third of the processedSavedRect.

    Rectangle {
        id: forwardx
        width: root.width * 0.05
        height: root.width
        anchors.centerIn: parent
        color: "red"
        rotation: 45
        visible: rating < 0
    }
    Rectangle {
        id: backwardX
        width: root.width * 0.05
        height: root.width
        anchors.centerIn: parent
        color: "red"
        rotation: -45
        visible: rating < 0
    }

    //Dots to show rating
    Rectangle {
        x: root.width * 0.1
        y: root.width * 0.13125
        width: root.width * 0.07
        height: root.width * 0.07
        color: "#DDCC33"
        border.color: "black"
        border.width: root.width * 0.007
        radius: width/2
        visible: rating >= 1
    }
    Rectangle {
        x: root.width * 0.2
        y: root.width * 0.13125
        width: root.width * 0.07
        height: root.width * 0.07
        color: "#DDCC33"
        border.color: "black"
        border.width: root.width * 0.007
        radius: width/2
        visible: rating >= 2
    }
    Rectangle {
        x: root.width * 0.3
        y: root.width * 0.13125
        width: root.width * 0.07
        height: root.width * 0.07
        color: "#DDCC33"
        border.color: "black"
        border.width: root.width * 0.007
        radius: width/2
        visible: rating >= 3
    }
    Rectangle {
        x: root.width * 0.4
        y: root.width * 0.13125
        width: root.width * 0.07
        height: root.width * 0.07
        color: "#DDCC33"
        border.color: "black"
        border.width: root.width * 0.007
        radius: width/2
        visible: rating >= 4
    }
    Rectangle {
        x: root.width * 0.5
        y: root.width * 0.13125
        width: root.width * 0.07
        height: root.width * 0.07
        color: "#DDCC33"
        border.color: "black"
        border.width: root.width * 0.007
        radius: width/2
        visible: rating >= 5
    }

    Component.onCompleted: {
        loadThumb.sourceComponent = thumbImage
        if (searchID == "") {
            console.log('queue delegate: model empty, but refreshing')
            //QueueModel.refreshAll()
            //QueueModel.updateAll()
            refresh()
        }
    }
}
