import QtQuick 2.3
import QtQuick.Controls 1.2
import QtQuick.Layouts 1.1
import QtWebKit 3.0
import "gui_components"

SplitView {
    id: root
    anchors.fill: parent
    orientation: Qt.Horizontal
    property real uiScale: 1

    property alias defaultExposureComp: editTools.defaultExposureComp
    property alias defaultWhitepoint: editTools.defaultWhitepoint
    property alias defaultBlackpoint: editTools.defaultBlackpoint
    property alias defaultShadowsY: editTools.defaultShadowsY
    property alias defaultHighlightsY: editTools.defaultHighlightsY
    property alias defaultFilmSize: editTools.defaultFilmSize
    property alias defaultHighlightRecovery: editTools.defaultHighlightRecovery
    property alias defaultLayerMixConst: editTools.defaultLayerMixConst
    property alias defaultCaEnabled: editTools.defaultCaEnabled
    property alias defaultTemperature: editTools.defaultTemperature
    property alias defaultTint: editTools.defaultTint
    property alias defaultVibrance: editTools.defaultVibrance
    property alias defaultSaturation: editTools.defaultSaturation
    property alias defaultOverdriveEnabled: editTools.defaultOverdriveEnabled
    property alias defaultHighlightRolloff: editTools.defaultHighlightRolloff

    signal tooltipWanted(string text, int x, int y)
    signal imageURL(string newURL)

    Rectangle {
        id: photoBox
        color: "black"
        Layout.fillWidth: true
        Flickable {
            id: flicky
            x: 0 * uiScale
            y: 30 * uiScale
            width: parent.width
            height: parent.height - 30 * uiScale
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
                color: "#000000"
                Image {
                    anchors.centerIn: parent
                    id: topImage
                    source: "image://filmy/" + indexString
                    fillMode: Image.PreserveAspectFit
                    asynchronous: true
                    mipmap: true
                    property int index: 0
                    property string indexString: "000000"
                    scale: bottomImage.scale
                    Connections {
                        target: paramManager//root
                        onUpdateImage: {
                            var num = (topImage.index + 1) % 1000000//1 in a million
                            topImage.index = num;
                            var s = num+"";
                            var size = 6 //6 digit number
                            while (s.length < size) {s = "0" + s}
                            topImage.indexString = s
                            console.log("Edit.qml; updateImage index: " + s)
                        }
                    }
                    onStatusChanged: {
                        if (topImage.status == Image.Ready) {
                            var topFitScaleX = flicky.width/topImage.width
                            var topFitScaleY = flicky.height/topImage.height
                            var topFitScale = Math.min(topFitScaleX, topFitScaleY)
                            flicky.sizeRatio = topFitScale / flicky.fitScale
                            console.log(flicky.sizeRatio)
                            console.log(bottomImage.scale)
                            //This isn't the right size ratio it seems.
                            if (flicky.sizeRatio > 1) {
                                //If the new image is smaller, we need to scale the bottom image up before selecting the image.
                                bottomImage.scale = bottomImage.scale * flicky.sizeRatio
                                bottomImage.source = topImage.source
                            }
                            else {
                                //If the new image is bigger, we want to select the image and then scale it down.
                                bottomImage.source = topImage.source
                                //bottomImage.scale = bottomImage.scale * sizeRatio
                            }
                            root.imageURL(topImage.source)
                        }
                    }
                }
                Image {
                    anchors.centerIn: parent
                    id: bottomImage
                    fillMode: Image.PreserveAspectFit
                    asynchronous: true
                    mipmap: true
                    onStatusChanged: {
                        if (bottomImage.status == Image.Ready) {
                            if (flicky.fit) {
                                //This is probably not necessary, but I don't want rounding errors to crop up.
                                bottomImage.scale = flicky.fitScale
                            }
                            else if (flicky.sizeRatio <= 1) {
                                bottomImage.scale = bottomImage.scale * flicky.sizeRatio
                            }
                            console.log(bottomImage.scale)
                        }
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
            id: filenameText
            x: 200 * uiScale
            y: 0 * uiScale
            color: "white"
            text: paramManager.filename
            font.pixelSize: 12.0 * uiScale
            elide: Text.ElideRight
        }
        Text {
            id: text2
            x: 200 * uiScale
            y: 15 * uiScale
            color: "white"
            text: paramManager.exposureTime + " s"
            font.pixelSize: 12.0 * uiScale
            elide: Text.ElideRight
        }
        Text {
            id: text3
            x: 350 * uiScale
            y: 0 * uiScale
            color: "white"
            text: "ISO " + paramManager.sensitivity
            font.pixelSize: 12.0 * uiScale
            elide: Text.ElideRight
        }
        Text {
            id: text4
            x: 350 * uiScale
            y: 15 * uiScale
            color: "white"
            text: "f/" + paramManager.aperture
            font.pixelSize: 12.0 * uiScale
            elide: Text.ElideRight
        }
    }
    EditTools {
        id: editTools
        uiScale: root.uiScale
        Component.onCompleted: {
            editTools.tooltipWanted.connect(root.tooltipWanted)
        }
    }
}
