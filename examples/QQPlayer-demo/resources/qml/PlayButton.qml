import QtQuick 2.15
import QtQuick.Controls 2.15

Button {
    id: button

    implicitWidth: 45
    implicitHeight: 45

    contentItem: Image {
        anchors.fill: parent
        source: button.down
                || button.hovered ? "qrc:/images/play_blue_light.png" : "qrc:/images/play_blue.png"
    }

    background: Item {}
}
