import QtQuick 2.2

Item {
    id: root
    property real dim
    width: dim
    height: dim

    property string rootDir

    property string searchID
    property int queueIndex
    property bool processed
    property bool exported

    property string __thumbPath: rootDir + '/' + searchID.slice(0,4) + '/' + searchID + '.jpg'

    signal openThisImage( string filePath, string sensitivity, string exposureTime, real aperture, real focalLength, real initialDeveloperConcentration, real reservoirThickness, real activeLayerThickness, real crystalsPerPixel, real initialCrystalRadius, real initialSilverSaltDensity, real developerConsumptionConst, real crystalGrowthConst, real silverSaltConsumptionConst, real totalDevelopmentTime, int agitateCount, int developmentResolution, real filmArea, real sigmaConst, real layerMixConst, real layerTimeDivisor, int rolloffBoundary, real exposureComp, real whitepoint, real blackpoint, real shadowsX, real shadowsY, real highlightsX, real highlightsY, int highlightRecovery, bool caEnabled, real temperature, real tint, real vibrance, real saturation, int orientation )

    Rectangle {
        anchors.fill: parent
        color: "lightsteelblue"
        opacity: ListView.isCurrentItem ? .3 : 0
        Text {
            y: 10
            color: "white"
            text: root.__thumbPath
        }
    }

    Loader {
        id: loadThumb
        asynchronous: true
        anchors.fill: parent
    }

    Component {
        id: thumbImage
        Image {
            id: thumb
            anchors.fill: parent
            fillMode: Image.PreserveAspectFit
            source: root.__thumbPath
        }
    }

    /*ListView.onIsCurrentItemChanged: {
        if ( ListView.isCurrentItem )
        openThisImage( filePath, sensitivity, exposureTime, aperture, focalLength, initialDeveloperConcentration, reservoirThickness, activeLayerThickness, crystalsPerPixel, initialCrystalRadius, initialSilverSaltDensity, developerConsumptionConst, crystalGrowthConst, silverSaltConsumptionConst, totalDevelopmentTime, agitateCount, developmentResolution, filmArea, sigmaConst, layerMixConst, layerTimeDivisor, rolloffBoundary, exposureComp, whitepoint, blackpoint, shadowsX, shadowsY, highlightsX, highlightsY, highlightRecovery, caEnabled, temperature, tint, vibrance, saturation, orientation )
    }*/

    Component.onCompleted: {
        loadThumb.sourceComponent = thumbImage
    }
}
