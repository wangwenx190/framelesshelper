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

    background: Rectangle {
        x: 0
        y: control.height / 2 - height / 2
        implicitWidth: 200
        implicitHeight: control.hovered ? control.hoveredHeight : control.normalHeight
        width: control.width
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

    handle: Item {}
}
