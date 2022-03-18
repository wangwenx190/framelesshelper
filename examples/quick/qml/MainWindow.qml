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

import QtQuick 2.12
import QtQuick.Window 2.12
import QtQuick.Controls 2.12
import org.wangwenx190.FramelessHelper 1.0

Window {
    id: window
    visible: true
    width: 800
    height: 600
    title: qsTr("Hello, World! - Qt Quick")
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
        color: window.active ? (FramelessUtils.titleBarColorVisible ? FramelessUtils.systemAccentColor
                                : (FramelessUtils.darkModeEnabled ? "black" : "white"))
                             : (FramelessUtils.darkModeEnabled ? "#202020" : "white")
        anchors {
            top: parent.top
            topMargin: windowTopBorder.height
            left: parent.left
            right: parent.right
        }

        Text {
            id: titleBarText
            text: window.title
            font.pointSize: 11
            color: window.active ? ((FramelessUtils.darkModeEnabled
                                     || FramelessUtils.titleBarColorVisible) ? "white" : "black") : "darkGray"
            anchors.left: parent.left
            anchors.leftMargin: 10
            anchors.verticalCenter: parent.verticalCenter
        }

        Row {
            anchors.top: parent.top
            anchors.right: parent.right

            MinimizeButton {
                id: minimizeButton
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
                id: closeButton
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

    Component.onCompleted: {
        FramelessHelper.addWindow(window);
        FramelessHelper.setTitleBarItem(window, titleBar);
        FramelessHelper.setHitTestVisible(window, minimizeButton, true);
        FramelessHelper.setHitTestVisible(window, maximizeButton, true);
        FramelessHelper.setHitTestVisible(window, closeButton, true);
    }
}
