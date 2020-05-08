import QtQuick 2.15
import QtQuick.Controls 2.15

Button {
    id: button

    width: 45
    height: 30

    ToolTip.visible: hovered && !down
    ToolTip.delay: Qt.styleHints.mousePressAndHoldInterval
    ToolTip.text: qsTr("Minimize")

    contentItem: Image {
        anchors.fill: parent
        source: "qrc:/images/button_minimize_black.svg"
    }

    background: Rectangle {
        visible: button.down || button.hovered
        color: button.down ? "#808080" : (button.hovered ? "#c7c7c7" : "transparent")
    }
}
