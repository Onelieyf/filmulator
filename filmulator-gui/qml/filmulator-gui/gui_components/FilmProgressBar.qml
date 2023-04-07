import QtQuick 2.3
import QtQuick.Controls 1.2
import QtQuick.Controls.Styles 1.2

Item {
    id: root
    property real uiScale: 1
    width: 200 * uiScale
    height: 30 * uiScale
    property alias value: progressBar.value
    property alias tooltipText: tooltip.tooltipText

    property real __padding: 2 * uiScale

    signal tooltipWanted(string text, int coordX, int coordY)

    ProgressBar {
        id: progressBar
        orientation: Qt.Horizontal
        indeterminate: false
        visible: true
        x: __padding
        y: __padding

        style: ProgressBarStyle {
            background: Rectangle {
                implicitWidth: root.width - __padding * 2
                implicitHeight: root.height - __padding * 2
                radius: 3 * uiScale
                color: "#B0B0B0"
                border.width: 1 * uiScale
                border.color: "#808080"
            }
            progress: Rectangle {
                color: "#FF9922"
                border.color: "#A87848"
                radius: 3 * uiScale
            }
        }
    }
    ToolTip {
        id: tooltip
        anchors.fill: root
        Component.onCompleted: {
            //forward tooltipWanted to root
            tooltip.tooltipWanted.connect(root.tooltipWanted)
        }
    }
}
