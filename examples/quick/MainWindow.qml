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
import QtQuick.Window 2.0
import QtQuick.Controls 2.0
import org.wangwenx190.FramelessHelper 1.0

FramelessWindow {
    id: window
    visible: true // Default is false, so won't be visible unless explicitly set to true.
    width: 800
    height: 600
    title: qsTr("FramelessHelper demo application - Qt Quick")
    color: (FramelessUtils.systemTheme === FramelessHelperConstants.Dark)
            ? FramelessUtils.defaultSystemDarkColor : FramelessUtils.defaultSystemLightColor

    Timer {
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
        color: (FramelessUtils.systemTheme === FramelessHelperConstants.Dark) ? "white" : "black"
    }

    StandardTitleBar {
        id: titleBar
        anchors {
            left: parent.left
            right: parent.right
        }
        active: window.active
        maximized: window.zoomed
        title: window.title
        minimizeButton {
            id: minimizeButton
            onClicked: window.showMinimized2()
        }
        maximizeButton {
            id: maximizeButton
            onClicked: window.toggleMaximized()
        }
        closeButton {
            id: closeButton
            onClicked: window.close()
        }
        Component.onCompleted: {
            // Make our homemade title bar snap to the window top frame border.
            window.snapToTopBorder(titleBar, FramelessHelperConstants.Top, FramelessHelperConstants.Bottom);
            // Make our homemade title bar draggable, and open the system menu
            // when the user right clicks on the title bar area.
            FramelessHelper.titleBarItem = titleBar;
            // Make our own items visible to the hit test and on Windows, enable
            // the snap layout feature (available since Windows 11).
            FramelessHelper.setSystemButton(minimizeButton, FramelessHelperConstants.Minimize);
            FramelessHelper.setSystemButton(maximizeButton, FramelessHelperConstants.Maximize);
            FramelessHelper.setSystemButton(closeButton, FramelessHelperConstants.Close);
        }
    }
}