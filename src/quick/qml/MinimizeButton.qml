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

Button {
    id: button
    implicitWidth: FramelessUtils.defaultSystemButtonSize.width
    implicitHeight: FramelessUtils.defaultSystemButtonSize.height
    contentItem: Item {
        implicitWidth: FramelessUtils.defaultSystemButtonIconSize.width
        implicitHeight: FramelessUtils.defaultSystemButtonIconSize.height

        Image {
            anchors.centerIn: parent
            source: (FramelessUtils.darkModeEnabled || FramelessUtils.titleBarColorized)
                    ? "image://framelesshelper/dark/minimize" : "image://framelesshelper/light/minimize"
        }
    }
    background: Rectangle {
        visible: button.hovered || button.pressed
        color: {
            if (button.pressed) {
                return FramelessUtils.defaultSystemButtonPressColor;
            }
            if (button.hovered) {
                return FramelessUtils.defaultSystemButtonHoverColor;
            }
            return "transparent";
        }
    }

    ToolTip {
        visible: button.hovered && !button.pressed
        delay: Qt.styleHints.mousePressAndHoldInterval
        text: qsTr("Minimize")
    }
}
