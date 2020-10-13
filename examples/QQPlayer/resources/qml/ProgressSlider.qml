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
import QtQuick.Controls 2.15

Slider {
    id: control
    topPadding: 0
    bottomPadding: 0
    leftPadding: 0
    rightPadding: 0

    property int progressDuration: 200
    property int heightDuration: 100
    property int hoveredHeight: 10
    property int normalHeight: 4
    property color backgroundColor: "#bdbebf"
    property color foregroundColor: "#21be2b"
    property color handleColor: "#f0f0f0"
    property int handleWidth: 14
    property int handleBorderWidth: 0
    property color handleBorderColor: "transparent"
    property bool handleVisibility: true

    background: Rectangle {
        x: 0
        y: control.availableHeight / 2 - height / 2
        implicitWidth: 200
        implicitHeight: control.hovered ? control.hoveredHeight : control.normalHeight
        width: control.availableWidth
        height: implicitHeight
        color: control.backgroundColor

        Behavior on implicitHeight {
            NumberAnimation {
                duration: control.heightDuration
            }
        }

        Rectangle {
            width: control.visualPosition * parent.width
            height: parent.height
            color: control.foregroundColor

            Behavior on width {
                NumberAnimation {
                    duration: control.progressDuration
                }
            }
        }
    }

    handle: Rectangle {
        visible: control.handleVisibility
        x: control.visualPosition * (control.availableWidth - width)
        y: control.availableHeight / 2 - height / 2
        implicitWidth: control.handleWidth
        implicitHeight: implicitWidth
        radius: implicitHeight / 2
        color: control.handleColor
        border.width: control.handleBorderWidth
        border.color: control.handleBorderColor

        Behavior on x {
            NumberAnimation {
                duration: control.progressDuration
            }
        }
    }
}
