import QtQuick 2.15
import QtQuick.Controls 2.15

Button {
    id: button

    implicitWidth: 45
    implicitHeight: 30

    property bool maximized: false

    ToolTip.visible: hovered && !down
    ToolTip.delay: Qt.styleHints.mousePressAndHoldInterval
    ToolTip.text: maximized ? qsTr("Restore") : qsTr("Maximize")

    contentItem: Image {
        anchors.fill: parent
        source: maximized ? "qrc:/images/button_restore_white.svg" : "qrc:/images/button_maximize_white.svg"
    }

    background: Rectangle {
        visible: button.down || button.hovered
        color: button.down ? "#808080" : (button.hovered ? "#c7c7c7" : "transparent")
    }
}
