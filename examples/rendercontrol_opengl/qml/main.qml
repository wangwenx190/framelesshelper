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
import QtQuick.Controls 2.0

Rectangle {
    id: root
    property real _flh_margin: ((window.visibility === Window.Maximized) || (window.visibility === Window.FullScreen)) ? 0 : 1
    property var _win_prev_state: null

    color: "#f0f0f0"

    Timer {
        id: timer
        interval: 500
        running: true
        repeat: true
        onTriggered: timeLabel.text = Qt.formatTime(new Date(), "hh:mm:ss")
    }

    Label {
        id: timeLabel
        anchors.centerIn: parent
        font {
            pointSize: 70
            bold: true
        }
    }

    Rectangle {
        id: titleBar
        height: window.titleBarHeight
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
                onClicked: {
                    console.log("minimizeButton clicked")
                    window.showMinimized()
                }
                Component.onCompleted: window.setHitTestVisible(minimizeButton, true)
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
                Component.onCompleted: window.setHitTestVisible(maximizeButton, true)
            }

            CloseButton {
                id: closeButton
                onClicked: window.close()
                Component.onCompleted: window.setHitTestVisible(closeButton, true)
            }
        }
    }

    Keys.onPressed: keyDown = true
    Keys.onReleased: keyDown = false
}
