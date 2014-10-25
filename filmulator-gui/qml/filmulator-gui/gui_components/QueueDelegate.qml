import QtQuick 2.2
import QtQuick.Controls 1.2
import QtQuick.Controls.Styles 1.2

Item {
    id: root
    property real dim
    width: dim * 0.9375
    height: dim

    property string rootDir

    property string selectedID
    property string searchID
    property int queueIndex
    property bool processed
    property bool exported

    property string __thumbPath: rootDir + '/' + searchID.slice(0,4) + '/' + searchID + '.jpg'

    signal openThisImage( string filePath, string sensitivity, string exposureTime, real aperture, real focalLength, real initialDeveloperConcentration, real reservoirThickness, real activeLayerThickness, real crystalsPerPixel, real initialCrystalRadius, real initialSilverSaltDensity, real developerConsumptionConst, real crystalGrowthConst, real silverSaltConsumptionConst, real totalDevelopmentTime, int agitateCount, int developmentResolution, real filmArea, real sigmaConst, real layerMixConst, real layerTimeDivisor, int rolloffBoundary, real exposureComp, real whitepoint, real blackpoint, real shadowsX, real shadowsY, real highlightsX, real highlightsY, int highlightRecovery, bool caEnabled, real temperature, real tint, real vibrance, real saturation, int orientation )

    Rectangle {
        id: currentImageRect
        width: root.dim*0.9375
        height: root.dim*0.03125
        color: searchID===selectedID ? "#FF8800" : "#00000000"
    }

    Loader {
        id: loadThumb
        asynchronous: true
    }

    Component {
        id: thumbImage
        Image {
            id: thumb
            x: 0
            y: root.height*0.03125
            width: root.width
            height: root.height*0.9375
            fillMode: Image.PreserveAspectFit
            source: root.__thumbPath
            sourceSize.width: 600
            sourceSize.height: 600
        }
    }

    Rectangle {
        id: processedSavedRect
        width: root.dim*0.9375
        height: root.dim*0.03125
        y: root.dim*0.96875
        color: exported ? "#00FF00" : (processed ? "#FF8800" : "#00000000")
    }

    Component.onCompleted: {
        loadThumb.sourceComponent = thumbImage
    }
}
