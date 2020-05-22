import QtQuick 2.15
import QtQuick.Controls 2.15

Button {
    id: control

    implicitWidth: 45
    implicitHeight: 30

    property bool maximized: false

    ToolTip.visible: hovered && !down
    ToolTip.delay: Qt.styleHints.mousePressAndHoldInterval
    ToolTip.text: maximized ? qsTr("Restore") : qsTr("Maximize")

    contentItem: Image {
        anchors.fill: control
        source: maximized ? "qrc:/images/button_restore_white.svg" : "qrc:/images/button_maximize_white.svg"
    }

    background: Rectangle {
        visible: control.down || control.hovered
        color: control.down ? "#808080" : (control.hovered ? "#c7c7c7" : "transparent")
    }
}
