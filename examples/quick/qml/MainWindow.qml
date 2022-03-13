/*
 * MIT License
 *
 * Copyright (C) 2022 by wangwenx190 (Yuhang Zhao)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

import QtQuick 2.15
import QtQuick.Window 2.15
import QtQuick.Controls 2.15
import org.wangwenx190.FramelessHelper 1.0

Window {
    id: window
    visible: true
    width: 800
    height: 600
    title: qsTr("Hello, World!")
    color: FramelessUtils.darkModeEnabled ? "#202020" : "#f0f0f0"

    Timer {
        id: timer
        interval: 500
        running: true
        repeat: true
        onTriggered: timeLabel.text = Qt.formatTime(new Date(), "hh:mm:ss")
    }

    Rectangle {
        id: titleBar
        height: 30
        color: window.active ? (FramelessUtils.titleBarColorVisible ? FramelessUtils.systemAccentColor :
                               (FramelessUtils.darkModeEnabled ? "white" : "black")) :
                               (FramelessUtils.darkModeEnabled ? "#202020" : "white")
        anchors {
            top: parent.top
            topMargin: windowTopBorder.height
            left: parent.left
            right: parent.right
        }

        Text {
            id: titleBarText
            text: window.title
            font.pointSize: 13
            color: window.active ? (FramelessUtils.darkModeEnabled ? "white" : "black") : "darkGray"
            anchors.left: parent.left
            anchors.leftMargin: 15
            anchors.verticalCenter: parent.verticalCenter
        }

        MouseArea {
            anchors.fill: parent
            anchors.rightMargin: 30 * 1.5 * 3
            acceptedButtons: Qt.LeftButton | Qt.RightButton
            hoverEnabled: true
            onClicked: {
                if (mouse.button === Qt.RightButton) {
                    FramelessUtils.showSystemMenu(window, Qt.point(mouse.x, mouse.y));
                }
            }
            onDoubleClicked: {
                if (mouse.button === Qt.LeftButton) {
                    maximizeButton.clicked();
                }
            }
            onPositionChanged: {
                if (containsPress && (window.visibility !== Window.FullScreen)) {
                    FramelessUtils.startSystemMove2(window);
                }
            }
        }

        Row {
            anchors.top: parent.top
            anchors.right: parent.right

            MinimizeButton {
                onClicked: FramelessUtils.showMinimized2(window)
            }

            MaximizeButton {
                id: maximizeButton
                maximized: ((window.visibility === Window.Maximized) || (window.visibility === Window.FullScreen))
                onClicked: {
                    if (maximized) {
                        window.showNormal();
                    } else {
                        window.showMaximized();
                    }
                }
            }

            CloseButton {
                onClicked: window.close()
            }
        }
    }

    Label {
        id: timeLabel
        anchors.centerIn: parent
        font {
            pointSize: 70
            bold: true
        }
        color: FramelessUtils.darkModeEnabled ? "white" : "black"
    }

    Rectangle {
        id: windowTopBorder
        anchors {
            top: parent.top
            left: parent.left
            right: parent.right
        }
        height: ((window.visibility === Window.Windowed) && FramelessUtils.frameBorderVisible) ? 1 : 0
        color: window.active ? FramelessUtils.frameBorderActiveColor : FramelessUtils.frameBorderInactiveColor
    }

    Component.onCompleted: FramelessHelper.addWindow(window)
}
