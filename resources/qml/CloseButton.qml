import QtQuick 2.15
import QtQuick.Controls 2.15

Button {
    id: button

    implicitWidth: 45
    implicitHeight: 30

    ToolTip.visible: hovered && !down
    ToolTip.delay: Qt.styleHints.mousePressAndHoldInterval
    ToolTip.text: qsTr("Close")

    contentItem: Image {
        anchors.fill: parent
        source: button.down
                || button.hovered ? "qrc:/images/button_close_white.svg" : "qrc:/images/button_close_black.svg"
    }

    background: Rectangle {
        visible: button.down || button.hovered
        color: button.down ? "#8c0a15" : (button.hovered ? "#e81123" : "transparent")
    }
}
