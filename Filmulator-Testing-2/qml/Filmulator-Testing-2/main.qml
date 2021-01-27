import QtQuick 2.2
import QtQuick.Controls 1.1
import QtQuick.Layouts 1.1
import QtQuick.Dialogs 1.1
import QtQuick.Controls.Styles 1.1

ApplicationWindow {
    id: mainwindow
    title: qsTr("Filmulator")
    minimumHeight: 600
    minimumWidth:800

    Rectangle {
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
            }

            Tab {
                title: qsTr("Organize")
                Organize {}
            }

            Tab {
                id: editortab
                property string location: ""
                property real rolling: 0
                property real exposureComp: 0
                property real whitepoint: 2/1000

                title: qsTr("Filmulate")
                Edit {
                    location: editortab.location
                    index: editortab.rolling
                    exposureComp: editortab.exposureComp
                    whitepoint: editortab.whitepoint
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
            Rectangle {
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
