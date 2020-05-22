import QtQuick 2.15
import QtQuick.Controls 2.15

Slider {
    id: control
    topPadding: 0
    bottomPadding: 0
    leftPadding: 0
    rightPadding: 0

    property int progressDuration: 200
    property int heightDuration: 100
    property int hoveredHeight: 10
    property int normalHeight: 4
    property color backgroundColor: "#bdbebf"
    property color foregroundColor: "#21be2b"
    property color handleColor: "#f0f0f0"
    property int handleWidth: 14
    property int handleBorderWidth: 0
    property color handleBorderColor: "transparent"
    property bool handleVisibility: true

    background: Rectangle {
        x: 0
        y: control.availableHeight / 2 - height / 2
        implicitWidth: 200
        implicitHeight: control.hovered ? control.hoveredHeight : control.normalHeight
        width: control.availableWidth
        height: implicitHeight
        color: control.backgroundColor

        Behavior on implicitHeight {
            NumberAnimation {
                duration: control.heightDuration
            }
        }

        Rectangle {
            width: control.visualPosition * parent.width
            height: parent.height
            color: control.foregroundColor

            Behavior on width {
                NumberAnimation {
                    duration: control.progressDuration
                }
            }
        }
    }

    handle: Rectangle {
        visible: control.handleVisibility
        x: control.visualPosition * (control.availableWidth - width)
        y: control.availableHeight / 2 - height / 2
        implicitWidth: control.handleWidth
        implicitHeight: implicitWidth
        radius: implicitHeight / 2
        color: control.handleColor
        border.width: control.handleBorderWidth
        border.color: control.handleBorderColor

        Behavior on x {
            NumberAnimation {
                duration: control.progressDuration
            }
        }
    }
}
