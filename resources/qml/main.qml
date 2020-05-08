import QtQuick 2.15

Item {
    id: root

    signal minimizeButtonClicked
    signal maximizeButtonClicked
    signal restoreButtonClicked
    signal closeButtonClicked

    Rectangle {
        id: titleBar
        height: $TitleBarHeight
        color: "white"
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right

        Text {
            id: titleBarText
            text: qsTr("Hello, World!")
            font.family: "Noto Sans CJK SC"
            font.pointSize: 15
            color: "black"
            anchors.left: parent.left
            anchors.leftMargin: 15
            anchors.verticalCenter: parent.verticalCenter
        }

        Row {
            anchors.top: parent.top
            anchors.right: parent.right

            MinimizeButton {
                onClicked: root.minimizeButtonClicked()
            }

            MaximizeButton {
                onClicked: {
                    if (maximized) {
                        root.restoreButtonClicked()
                        maximized = false
                    } else {
                        root.maximizeButtonClicked()
                        maximized = true
                    }
                }
            }

            CloseButton {
                onClicked: root.closeButtonClicked()
            }
        }
    }

    Rectangle {
        id: content
        color: "#f0f0f0"
        anchors.top: titleBar.bottom
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.right: parent.right
    }
}
