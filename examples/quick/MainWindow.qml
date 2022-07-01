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

import QtQuick
import QtQuick.Controls.Basic
import org.wangwenx190.FramelessHelper
import Demo

FramelessWindow {
    id: window
    visible: false // Hide the window before we sets up it's correct size and position.
    width: 800
    height: 600
    title: qsTr("FramelessHelper demo application - Qt Quick")
    color: (FramelessUtils.systemTheme === FramelessHelperConstants.Dark)
            ? FramelessUtils.defaultSystemDarkColor : FramelessUtils.defaultSystemLightColor
    onClosing: Settings.saveGeometry(window)

    FramelessHelper.onReady: {
        // Let FramelessHelper know what's our homemade title bar, otherwise
        // our window won't be draggable.
        FramelessHelper.titleBarItem = titleBar;
        // Make our own items visible to the hit test and on Windows, enable
        // the snap layouts feature (available since Windows 11).
        FramelessHelper.setSystemButton(titleBar.minimizeButton, FramelessHelperConstants.Minimize);
        FramelessHelper.setSystemButton(titleBar.maximizeButton, FramelessHelperConstants.Maximize);
        FramelessHelper.setSystemButton(titleBar.closeButton, FramelessHelperConstants.Close);
        if (!Settings.restoreGeometry(window)) {
            FramelessHelper.moveWindowToDesktopCenter();
        }
        // Finally, show the window after everything is setted.
        window.visible = true;
    }

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
            top: window.topBorderBottom // VERY IMPORTANT!
            left: parent.left
            right: parent.right
        }
    }
}
