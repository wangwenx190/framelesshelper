import QtQuick 2.15
import QtQuick.Controls 2.15

Button {
    id: control

    implicitWidth: 14
    implicitHeight: 14

    contentItem: Image {
        anchors.fill: control
        source: control.down
                || control.hovered ? "qrc:/images/stop_blue_light.png" : "qrc:/images/stop_blue.png"
    }

    background: Item {}
}
