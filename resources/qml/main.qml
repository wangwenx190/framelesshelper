import QtQuick 2.15
import QtQuick.Window 2.15
import wangwenx190.Utils 1.0

Window {
    id: window
    visible: true
    width: 800
    height: 600
    title: qsTr("Hello, World!")

    FramelessHelper {
        id: framelessHelper
    }

    Rectangle {
        anchors.fill: parent
        border.color: Qt.application.state === Qt.ApplicationActive ? "#707070" : "#aaaaaa"
    }

    Rectangle {
        id: titleBar
        height: framelessHelper.titleBarHeight
        color: "white"
        anchors {
            top: parent.top
            topMargin: 1
            left: parent.left
            leftMargin: 1
            right: parent.right
            rightMargin: 1
        }

        Text {
            id: titleBarText
            text: window.title
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
                id: minimizeButton
                onClicked: window.showMinimized()
                Component.onCompleted: framelessHelper.addIgnoreObject(
                                           minimizeButton)
            }

            MaximizeButton {
                id: maximizeButton
                maximized: window.visibility === Window.Maximized
                onClicked: {
                    if (maximized) {
                        window.showNormal()
                    } else {
                        window.showMaximized()
                    }
                }
                Component.onCompleted: framelessHelper.addIgnoreObject(
                                           maximizeButton)
            }

            CloseButton {
                id: closeButton
                onClicked: window.close()
                Component.onCompleted: framelessHelper.addIgnoreObject(
                                           closeButton)
            }
        }
    }

    Rectangle {
        id: content
        color: "#f0f0f0"
        anchors {
            top: titleBar.bottom
            bottom: parent.bottom
            bottomMargin: 1
            left: parent.left
            leftMargin: 1
            right: parent.right
            rightMargin: 1
        }
    }

    Component.onCompleted: framelessHelper.removeWindowFrame()
}
