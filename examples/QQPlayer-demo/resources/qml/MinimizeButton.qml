import QtQuick 2.15
import QtQuick.Controls 2.15

Button {
    id: control

    implicitWidth: 45
    implicitHeight: 30

    ToolTip.visible: hovered && !down
    ToolTip.delay: Qt.styleHints.mousePressAndHoldInterval
    ToolTip.text: qsTr("Minimize")

    contentItem: Image {
        anchors.fill: control
        source: "qrc:/images/button_minimize_white.svg"
    }

    background: Rectangle {
        visible: control.down || control.hovered
        color: control.down ? "#808080" : (control.hovered ? "#c7c7c7" : "transparent")
    }
}
