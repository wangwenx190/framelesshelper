import QtQuick 2.15
import QtQuick.Controls 2.15

Button {
    id: control

    implicitWidth: 75
    implicitHeight: 15

    contentItem: Image {
        anchors.fill: control
        source: control.down
                || control.hovered ? "qrc:/images/main_menu_blue.png" : "qrc:/images/main_menu_white.png"
    }

    background: Item {}
}
