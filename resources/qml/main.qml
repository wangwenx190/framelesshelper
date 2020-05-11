import QtQuick 2.15
import QtQuick.Window 2.15

Window {
    id: root
    visible: true
    width: 800
    height: 600
    title: qsTr("Hello, World!")

    Rectangle {
        id: titleBar
        height: 30
        color: "white"
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right

        Text {
            id: titleBarText
            text: root.title
            font.family: "Noto Sans CJK SC"
            font.pointSize: 13
            color: "black"
            anchors.left: parent.left
            anchors.leftMargin: 15
            anchors.verticalCenter: parent.verticalCenter
        }

        Row {
            anchors.top: parent.top
            anchors.right: parent.right

            MinimizeButton {
                onClicked: root.showMinimized()
            }

            MaximizeButton {
                maximized: root.visibility === 4
                onClicked: {
                    if (maximized) {
                        root.showNormal()
                    } else {
                        root.showMaximized()
                    }
                }
            }

            CloseButton {
                onClicked: root.close()
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
