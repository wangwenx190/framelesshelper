import QtQuick 2.15
import QtQuick.Window 2.15
import wangwenx190.Utils 1.0

Window {
    id: window
    visible: true
    width: 800
    height: 540
    title: qsTr("QQ Player")

    property color themeColor: "#111111"

    FramelessHelper {
        id: framelessHelper
    }

    Rectangle {
        id: titleBar
        height: framelessHelper.titleBarHeight
        color: window.themeColor
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right

        MainMenuButton {
            id: mainMenuButton
            anchors.left: parent.left
            anchors.leftMargin: 15
            anchors.verticalCenter: parent.verticalCenter
            Component.onCompleted: framelessHelper.addIgnoreObject(
                                       mainMenuButton)
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

    Image {
        id: backgroundImage
        anchors.top: titleBar.bottom
        anchors.bottom: controlPanel.top
        anchors.left: parent.left
        anchors.right: parent.right
        source: "qrc:/images/background.png"

        Image {
            id: logoImage
            width: 123
            height: 99
            anchors.centerIn: parent
            source: "qrc:/images/logo.png"
        }

        OpenFileButton {
            id: openFileButton
            anchors.top: logoImage.bottom
            anchors.topMargin: 25
            anchors.horizontalCenter: parent.horizontalCenter
        }
    }

    Rectangle {
        id: controlPanel
        height: 63
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        color: window.themeColor

        ProgressSlider {
            id: progressSlider
            anchors.bottom: parent.top
            anchors.bottomMargin: -2
            anchors.left: parent.left
            anchors.right: parent.right
            foregroundColor: "#1ab7e4"
            handleColor: "#923cf2"
        }

        StopButton {
            id: stopButton
            anchors.right: previousButton.left
            anchors.rightMargin: 30
            anchors.verticalCenter: parent.verticalCenter
        }

        PreviousButton {
            id: previousButton
            anchors.right: playButton.left
            anchors.rightMargin: 30
            anchors.verticalCenter: parent.verticalCenter
        }

        PlayButton {
            id: playButton
            anchors.centerIn: parent
        }

        NextButton {
            id: nextButton
            anchors.left: playButton.right
            anchors.leftMargin: 30
            anchors.verticalCenter: parent.verticalCenter
        }

        VolumeButton {
            id: volumeButton
            anchors.left: nextButton.right
            anchors.leftMargin: 30
            anchors.verticalCenter: parent.verticalCenter
        }
    }

    Component.onCompleted: framelessHelper.removeWindowFrame()
}
