import QtQuick 2.15
import QtQuick.Controls 2.15

Button {
    id: button

    width: 45
    height: 45

    contentItem: Image {
        anchors.fill: parent
        source: button.down
                || button.hovered ? "qrc:/images/play_blue_light.png" : "qrc:/images/play_blue.png"
    }

    background: Item {}
}
