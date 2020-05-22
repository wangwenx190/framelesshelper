import QtQuick 2.15
import QtQuick.Controls 2.15

Button {
    id: control

    implicitWidth: 187
    implicitHeight: 50

    contentItem: Image {
        anchors.fill: control
        source: control.down
                || control.hovered ? "qrc:/images/open_file_blue.png" : "qrc:/images/open_file_white.png"
    }

    background: Item {}
}
