import QtQuick 2.3
import QtQuick.Controls 1.2
import QtQuick.Controls.Styles 1.2
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
    property bool rightClicked: false
    z: rightClicked ? 1 : 0

    property string __thumbPath: rootDir + '/' + searchID.slice(0,4) + '/' + searchID + '.jpg'

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

    Rectangle {
        id: currentImageRect
        width: root.width
        height: root.height*0.03125
        color: rightClicked ? Colors.whiteOrange : (__current ? Colors.medOrange : "#00000000")
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
                    onThumbnailDone: {
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

    Component.onCompleted: {
        loadThumb.sourceComponent = thumbImage
    }
}
