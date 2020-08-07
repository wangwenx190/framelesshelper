import QtQuick 2.15
import QtQuick.Window 2.15
import wangwenx190.Utils 1.0

Window {
    id: window
    visible: true
    width: 800
    height: 600
    title: qsTr("Hello, World!")

    property bool isWin10OrGreater: false

    FramelessHelper {
        id: framelessHelper
    }

    Rectangle {
        visible: isWin10OrGreater && (window.visibility === Window.Windowed)
        anchors {
            top: parent.top
            left: parent.left
            right: parent.right
        }
        height: 1
        color: Qt.application.state === Qt.ApplicationActive ? /*"#707070"*/ "#ffffff" : "#aaaaaa"
    }

    Rectangle {
        id: titleBar
        height: framelessHelper.titleBarHeight
        color: "white"
        anchors {
            top: parent.top
            topMargin: 1
            left: parent.left
            right: parent.right
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
            left: parent.left
            right: parent.right
        }
    }

    Component.onCompleted: framelessHelper.removeWindowFrame()
}
