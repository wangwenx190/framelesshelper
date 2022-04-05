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

import QtQuick 2.0
import QtQuick.Controls 2.0
import org.wangwenx190.FramelessHelper 1.0

Rectangle {
    property bool active: true
    property bool maximized: false
    property alias title: windowTitleLabel.text
    property alias minimizeButton: minimizeButton
    property alias maximizeButton: maximizeButton
    property alias closeButton: closeButton

    id: titleBar
    height: FramelessUtils.titleBarHeight
    color: titleBar.active ? (FramelessUtils.titleBarColorized ? FramelessUtils.systemAccentColor
              : (FramelessUtils.darkModeEnabled ? "black" : "white"))
              : (FramelessUtils.darkModeEnabled ? FramelessUtils.defaultSystemDarkColor : "white")

    Text {
        id: windowTitleLabel
        font.pointSize: 11
        color: titleBar.active ? ((FramelessUtils.darkModeEnabled
                  || FramelessUtils.titleBarColorized) ? "white" : "black") : "darkGray"
        anchors {
            left: parent.left
            leftMargin: 10
            verticalCenter: parent.verticalCenter
        }
    }

    Row {
        anchors {
            top: parent.top
            right: parent.right
        }

        StandardMinimizeButton {
            id: minimizeButton
        }

        StandardMaximizeButton {
            id: maximizeButton
            maximized: titleBar.maximized
        }

        StandardCloseButton {
            id: closeButton
        }
    }
}
