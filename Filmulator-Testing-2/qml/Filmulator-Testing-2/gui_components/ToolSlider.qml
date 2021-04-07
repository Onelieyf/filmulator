import QtQuick 2.1
import QtQuick.Controls 1.1
import QtQuick.Controls.Styles 1.1

Rectangle {
    id: toolSlider
    implicitHeight: 30
    implicitWidth: parent.width
    property alias title: label.text
    property alias minimumValue: slider.minimumValue
    property alias maximumValue: slider.maximumValue
    property alias updateValueWhileDragging: slider.updateValueWhileDragging
    property alias stepSize: slider.stepSize
    property alias value: slider.value
    property real defaultValue
    property alias valueText: valueText.text

    property alias tickmarksEnabled: slider.tickmarksEnabled

    property real __padding: 2

    color: "#303030"

    Text {
        id: label
        color: "white"
        width: parent.width - valueBox.width - reset.width- 2*__padding
        x: __padding
        y: __padding
        elide: Text.ElideRight
    }
    Rectangle {
        id: valueBox
        color: "black"
        width: 60
        height: 20 - __padding
        x: parent.width - this.width - 30 - __padding
        y: __padding
        Text {
            id: valueText
            anchors.fill: parent
            color: "white"
            text: slider.value
            elide: Text.ElideRight
        }
    }

    Slider {
        id: slider
        x: __padding
        y: 20
        width: parent.width - 2*__padding - 30
        updateValueWhileDragging: true
        value: defaultValue
        style: SliderStyle {
            groove: Rectangle {
                height: 4
                color: "#FF8800"
            }
            handle: Rectangle {
                height: 8
                width: 20
                radius: 3
                color: control.pressed ? "#A0A0A0" : "#808080"
            }
        }
    }
    Button {
        id: reset
        width: 26
        height: 26
        x: toolSlider.width-width-__padding
        y: __padding
        text: "[]"
        action: Action {
            onTriggered: {
                slider.value = defaultValue
            }
        }

        style: ButtonStyle {
            background: Rectangle {
                implicitWidth: 26
                implicitHeight: 26
                border.width: 2
                border.color: "#202020"
                radius: 5
                color: control.pressed ? "#A0A0A0" : "#808080"
            }
            label: Text{
                color: "white"
                anchors.centerIn: parent
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                text: control.text
            }
        }
    }
    MouseArea {
        id: rightclickreset
        acceptedButtons: Qt.RightButton
        anchors.fill: slider
        onDoubleClicked: {
            slider.value = defaultValue
        }
    }
}
