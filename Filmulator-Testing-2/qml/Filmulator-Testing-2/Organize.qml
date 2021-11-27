import QtQuick 2.2
import QtQuick.Controls 1.2
import QtQuick.Layouts 1.1
import "gui_components"

SplitView {
    id: root
    anchors.fill: parent
    orientation: Qt.Horizontal

    Rectangle {
        id: folderlist
        color: "lightblue"
        width: 100
        Layout.maximumWidth: 500
    }

    Rectangle {
        id: gridViewBox
        color: "#202020"
        Layout.fillWidth: true

        GridView {
            id: gridView
            anchors.fill: parent

            cellWidth: 400
            cellHeight: 300

            delegate: OrganizeDelegate {hour: searchID}
            Component.onCompleted: {
                organizeModel.setOrganizeQuery()
                organizeModel.minCaptureTime = 0
                organizeModel.maxCaptureTime = 1400000000
                organizeModel.minImportTime = 0
                organizeModel.maxImportTime = 1400000000
                organizeModel.minProcessedTime = 0
                organizeModel.maxProcessedTime = 1400000000
                gridView.model = organizeModel
            }
        }
    }
}
