import QtQuick 2.15
import QtQuick.Controls 2.15

Button {
    id: button

    implicitWidth: 18
    implicitHeight: 14

    contentItem: Image {
        anchors.fill: parent
        source: button.down
                || button.hovered ? "qrc:/images/volume_blue_light.png" : "qrc:/images/volume_blue.png"
    }

    background: Item {}
}
