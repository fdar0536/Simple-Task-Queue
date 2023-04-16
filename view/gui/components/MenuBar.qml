/*
 * Simple Task Queue
 * Copyright (c) 2023 fdar0536
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
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
import QtQuick.Controls
import QtQuick.Controls.Material

ToolBar
{
    id: root
    signal menuClicked()
    signal infoClicked()
    signal closeClicked()

    Row
    {
        ToolbarIcon
        {
            id: menuBtn
            source: "qrc:/view/gui/icons/menu.svg"
            toolTip: qsTr("Menu")
            onClicked: root.menuClicked()
        }

        ToolbarIcon
        {
            id: infoBtn
            source: "qrc:/view/gui/icons/info.svg"
            toolTip: qsTr("About Qt")
            onClicked: root.infoClicked()
        }

        ToolbarIcon
        {
            id: closeBtn
            source: "qrc:/view/gui/icons/close.svg"
            toolTip: qsTr("Exit")
            onClicked: root.closeClicked()
        }
    }
}