import QtQuick 2.2
import QtQuick.Controls 1.1
import QtQuick.Layouts 1.1
import QtQuick.Dialogs 1.1
import QtQuick.Controls.Styles 1.1
import "gui_components"

ApplicationWindow {
    id: root
    title: qsTr("Filmulator")
    width: 1024
    height: 768
    minimumHeight: 600
    minimumWidth:800

    signal tooltipWanted(string text, int x, int y)

    Rectangle {
        id: fillRect
        anchors.fill: parent
        color: "#FF303030"
    }

    SplitView {
        anchors.fill: parent
        orientation: Qt.Vertical

        TabView {
            anchors.margins: Qt.platform.os === "osx" ? 12 : 2
            tabPosition: Qt.TopEdge
            Layout.fillHeight: true
            Layout.minimumHeight: 300
            style: headerTabViewStyle

            Tab {
                title: qsTr("Import")
                Import {}
            }

            Tab {
                title: qsTr("Organize")
                Organize {}
            }

            Tab {
                id: editorTab
                property string location: ""
                property real exposureComp: 0
                property real defaultExposureComp: 0
                property real whitepoint: 2/1000
                property real defaultWhitepoint: 2/1000
                property real blackpoint: 0
                property real defaultBlackpoint: 0
                property real shadowsY: 0.25
                property real defaultShadowsY: 0.25
                property real highlightsY: 0.75
                property real defaultHighlightsY: 0.75
                property real filmSize: 864
                property real defaultFilmSize: 864
                property bool defaultCurve: true
                property int highlightRecovery: 0
                property int defaultHighlightRecovery: 0
                property real layerMixConst: 0.2
                property real defaultLayerMixConst: 0.2

                title: qsTr("Filmulate")
                Edit {
                    id: editItem
                    location: editorTab.location
                    exposureComp: editorTab.exposureComp
                    defaultExposureComp: editorTab.defaultExposureComp
                    filmSize: editorTab.filmSize
                    defaultFilmSize: editorTab.defaultFilmSize
                    whitepoint: editorTab.whitepoint
                    defaultWhitepoint: editorTab.defaultWhitepoint
                    blackpoint: editorTab.blackpoint
                    defaultBlackpoint: editorTab.defaultBlackpoint
                    shadowsY: editorTab.shadowsY
                    defaultShadowsY: editorTab.defaultShadowsY
                    highlightsY: editorTab.highlightsY
                    defaultHighlightsY: editorTab.defaultHighlightsY
                    defaultCurve: editorTab.defaultCurve
                    highlightRecovery: editorTab.highlightRecovery
                    defaultHighlightRecovery: editorTab.defaultHighlightRecovery
                    layerMixConst: editorTab.layerMixConst
                    defaultLayerMixConst: editorTab.defaultLayerMixConst

                    Connections {
                        target: openDialog
                        onAccepted: reset()
                    }
                    Component.onCompleted: {
                        editItem.tooltipWanted.connect(root.tooltipWanted)
                    }
                }
            }

            Tab {
                title: qsTr("Output")
            }
        }

        Rectangle {
            id: queue
            color: "lightgreen"
            height: 100
            Layout.minimumHeight: 50

            ToolButton {
                id: openButton
                anchors.centerIn: parent
                text: qsTr("Open")
                width: 80
                height: 40
                action: Action {
                    onTriggered: {
                        openDialog.open()
                    }
                }
            }

            FileDialog {
                id: openDialog
                title: qsTr("Select a raw file")
                onAccepted: {
                    editorTab.location = fileUrl
                    /*
                    filmProvider.defaultToneCurveEnabled = editortab.defaultCurve
                    filmProvider.exposureComp = editortab.exposureComp
                    filmProvider.whitepoint = editortab.whitepoint
                    filmProvider.blackpoint = editortab.blackpoint
                    filmProvider.shadowsY = editortab.shadowsY
                    filmProvider.highlightsY = editortab.highlightsY
                    filmProvider.filmArea = editortab.filmSize
                    */
                }
            }

            /*            Rectangle {
                id: textentryholder
                color: "#101010"
                height: 20
                width: 300
                radius: 5
                anchors.centerIn: parent
                TextInput {
                    id: filelocation
                    color: "#FFFFFF"
                    anchors.fill: parent
                    text: qsTr("Enter file path here")
                    onAccepted: {
                        editortab.location = filelocation.text
                        filmProvider.exposureComp = editortab.exposureComp;
                        filmProvider.whitepoint = editortab.whitepoint;
                    }
                }
            }*/
        }
    }


    //Tooltip handling
    onTooltipWanted: {
        tooltipText.text = text
        tooltipCatcher.xInput = x
        tooltipCatcher.yInput = y
        tooltipCatcher.enabled = true
        tooltipCatcher.visible = true
    }

    MouseArea {
        id: tooltipCatcher
        acceptedButtons: Qt.NoButton
        anchors.fill: fillRect
        hoverEnabled: true
        enabled: false
        visible: false
        propagateComposedEvents: true
        property int xInput
        property int yInput
        property Item sourceItem

        onEnabledChanged: {
            if (enabled) {
                tooltipCatcher.setPosition(xInput, yInput)
                //tooltipTimer.start()
            }
        }

        onPositionChanged: {
            tooltipCatcher.visible = false
            tooltipCatcher.enabled = false
        }
        onExited: {
            tooltipCatcher.visible = false
            tooltipCatcher.enabled = false
        }

/*        Timer {
            id: tooltipTimer
            interval: 5000
            onTriggered: {
                tooltipCatcher.visible = false
                tooltipCatcher.enabled = false
            }
        }*/

        Rectangle {
            id: tooltipBox
            color: "#EE303030"
            border.color: "#EE808080"
            border.width: 2
            radius: 10
            property int padding: 6
            property int maxWidth: 250
            property int minHeight: 30
            property int posPad: 10
            width: Math.min(maxWidth,tooltipText.contentWidth+2*padding)
            height: tooltipText.contentHeight+2*padding
            Text {
                id: tooltipText
                x: parent.padding
                y: parent.padding
                width: parent.maxWidth-2*parent.padding
                wrapMode: Text.WordWrap
                color: "#FFFFFFFF"
            }
        }
        function setPosition(xIn, yIn) {
            if (tooltipBox.height + yIn < root.height) {
                tooltipBox.y = yIn
            }
            else {
                tooltipBox.y = yIn - tooltipBox.height
            }
            if (tooltipBox.width + xIn + tooltipBox.posPad < root.width) {
                tooltipBox.x = xIn + tooltipBox.posPad
            }
            else {
                tooltipBox.x = xIn - tooltipBox.width
            }
        }
    }

    //styles
    property Component headerTabViewStyle: TabViewStyle {
        tabOverlap: -4
        frameOverlap: -4

        frame: Rectangle {
            gradient: Gradient {
                GradientStop { color:"#505050"; position: 0 }
                GradientStop { color:"#000000"; position: 0 }
            }
        }

        tab: Rectangle {
            property int totalOverlap: tabOverlap * (control.count - 1)
            implicitWidth: Math.min ((styleData.availableWidth + totalOverlap)/control.count - 4, 100)
            implicitHeight: 30
            radius: 8
            border.color: styleData.selected ? "#B0B0B0" : "#808080"
            color: "#111111"
            Text {
                text: styleData.title
                color: "#ffffff"
                anchors.centerIn: parent
                font.bold: true
            }
        }
    }
}
