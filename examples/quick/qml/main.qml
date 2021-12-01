/*
 * MIT License
 *
 * Copyright (C) 2021 by wangwenx190 (Yuhang Zhao)
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

import QtQuick 2.0
import QtQuick.Window 2.0
import QtQuick.Controls 2.0
import wangwenx190.Utils 1.0

Window {
    id: window
    visible: true
    width: 800
    height: 600
    title: qsTr("Hello, World!")
    color: "#f0f0f0"

    property real _flh_margin: ((window.visibility === Window.Maximized) | (window.visibility === Window.FullScreen)) ? 0.0 : 1.0
    property var _win_prev_state: null

    FramelessHelper {
        id: framelessHelper
    }

    Timer {
        id: timer
        interval: 500
        running: true
        repeat: true
        onTriggered: timeLabel.text = Qt.formatTime(new Date(), "hh:mm:ss")
    }

    Rectangle {
        id: titleBar
        height: framelessHelper.titleBarHeight
        color: "white"
        anchors {
            top: parent.top
            topMargin: window._flh_margin
            left: parent.left
            leftMargin: window._flh_margin
            right: parent.right
            rightMargin: window._flh_margin
        }

        Text {
            id: titleBarText
            text: window.title
            font.pointSize: 13
            color: window.active ? "black" : "gray"
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
                Component.onCompleted: framelessHelper.setHitTestVisible(minimizeButton, true)
            }

            MaximizeButton {
                id: maximizeButton
                maximized: ((window.visibility === Window.Maximized) || (window.visibility === Window.FullScreen))
                onClicked: {
                    if (maximized) {
                        window.showNormal()
                    } else {
                        window.showMaximized()
                    }
                }
                Component.onCompleted: framelessHelper.setHitTestVisible(maximizeButton, true)
            }

            CloseButton {
                id: closeButton
                onClicked: window.close()
                Component.onCompleted: framelessHelper.setHitTestVisible(closeButton, true)
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
    }

    Button {
        id: fullScreenButton
        anchors {
            horizontalCenter: parent.horizontalCenter
            top: timeLabel.bottom
            topMargin: 15
        }
        property bool _full: window.visibility === Window.FullScreen
        text: _full ? qsTr("Exit FullScreen") : qsTr("Enter FullScreen")
        onClicked: {
            if (_full) {
                if (_win_prev_state == Window.Maximized) {
                    window.showMaximized()
                } else if (_win_prev_state == Window.Windowed) {
                    window.showNormal()
                }
            } else {
                _win_prev_state = window.visibility
                window.showFullScreen()
            }
        }
    }

    Rectangle {
        id: windowFrame
        anchors.fill: parent
        color: "transparent"
        border {
            color: window.active ? "black" : "darkGray"
            width: window._flh_margin
        }
    }

    Component.onCompleted: framelessHelper.removeWindowFrame()
}
