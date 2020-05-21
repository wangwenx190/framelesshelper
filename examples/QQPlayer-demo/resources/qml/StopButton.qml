import QtQuick 2.15
import QtQuick.Controls 2.15

Button {
    id: button

    width: 14
    height: 14

    contentItem: Image {
        anchors.fill: parent
        source: button.down
                || button.hovered ? "qrc:/images/stop_blue_light.png" : "qrc:/images/stop_blue.png"
    }

    background: Item {}
}
