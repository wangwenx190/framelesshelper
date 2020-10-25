/*
 * MIT License
 *
 * Copyright (C) 2020 by wangwenx190 (Yuhang Zhao)
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
import QtQuick.Layouts 1.15
import wangwenx190.Utils 1.0

Window {
    id: window
    visible: true
    width: 800
    height: 600
    title: qsTr("Hello, World!")
    color: blurEffectCheckBox.checked ? "transparent" : "#f0f0f0"

    FramelessHelper {
        id: framelessHelper
    }

    Rectangle {
        id: titleBar
        height: framelessHelper.titleBarHeight
        color: extendToTitleBarCheckBox.checked ? "transparent" : ((window.active && framelessHelper.colorizationEnabled) ? framelessHelper.colorizationColor : "white")
        anchors {
            top: parent.top
            left: parent.left
            right: parent.right
        }

        Label {
            id: titleBarText
            text: window.title
            font.pointSize: 13
            color: window.active ? (framelessHelper.colorizationEnabled ? "white" : "black") : "gray"
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
        color: "transparent"
        anchors {
            top: titleBar.bottom
            bottom: parent.bottom
            left: parent.left
            right: parent.right
        }

        ColumnLayout {
            anchors.centerIn: parent
            spacing: 25

            Label {
                Layout.alignment: Qt.AlignCenter
                font.pointSize: 15
                font.bold: true
                text: qsTr("Current system theme: %1").arg(framelessHelper.darkThemeEnabled ? "dark theme" : "light theme")
            }

            Label {
                Layout.alignment: Qt.AlignCenter
                font.pointSize: 15
                font.bold: true
                text: qsTr("Transparency effect: %1").arg(framelessHelper.transparencyEffectEnabled ? "enabled" : "disabled")
            }

            CheckBox {
                id: blurEffectCheckBox
                Layout.alignment: Qt.AlignCenter
                font.pointSize: 15
                font.bold: true
                text: qsTr("Enable blur effect")
                onCheckedChanged: framelessHelper.setBlurEffectEnabled(checked, forceAcrylicCheckBox.checked)
            }

            CheckBox {
                id: forceAcrylicCheckBox
                Layout.alignment: Qt.AlignCenter
                font.pointSize: 15
                font.bold: true
                text: qsTr("Force enabling Acrylic effect")
                enabled: framelessHelper.canHaveWindowFrame
            }

            CheckBox {
                id: extendToTitleBarCheckBox
                Layout.alignment: Qt.AlignCenter
                font.pointSize: 15
                font.bold: true
                text: qsTr("Extend to title bar")
            }

            Button {
                Layout.alignment: Qt.AlignCenter
                text: qsTr("Move to desktop center")
                font.pointSize: 15
                font.bold: true
                onClicked: framelessHelper.moveWindowToDesktopCenter(true)
            }
        }
    }

    Rectangle {
        id: topFrame
        visible: framelessHelper.canHaveWindowFrame && (window.visibility === Window.Windowed)
        color: window.active ? (framelessHelper.colorizationEnabled ? framelessHelper.colorizationColor : "#707070") : "#aaaaaa"
        anchors {
            top: parent.top
            left: parent.left
            right: parent.right
        }
        height: 1
    }

    Component.onCompleted: {
        framelessHelper.setWindowFrameVisible(framelessHelper.canHaveWindowFrame)
        framelessHelper.removeWindowFrame(true)
    }
}
