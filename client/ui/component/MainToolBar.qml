/*
 * Simple Task Queue
 * Copyright (c) 2022 fdar0536
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

import QtQuick 2.15
import QtQuick.Controls
import QtQuick.Controls.Material 2.12

ToolBar
{
    id: root
    signal menuClicked()
    signal backClicked()

    anchors
    {
        top: parent.top
        left: parent.left
        right: parent.right
    }

    Image
    {
        id: menu
        anchors
        {
            top: parent.top
            left: parent.left
            bottom: parent.bottom
        }

        source: "qrc:/ui/icon/menu_black_48dp.svg"
        transformOrigin: Item.Center

        MouseArea
        {
            anchors.fill: parent

            onClicked: function()
            {
                if (menu.rotation === 0)
                {
                    menu.rotation = 90
                }
                else
                {
                    menu.rotation = 0
                }

                root.menuClicked()
            } // end onClicked
        } // end MouseArea
    } // end Image menu

    Image
    {
        id: back
        anchors
        {
            top: parent.top
            left: menu.right
            bottom: parent.bottom
        }

        source: "qrc:/ui/icon/arrow_back_black_48dp.svg"
        transformOrigin: Item.Center

        MouseArea
        {
            anchors.fill: parent
            onClicked: root.backClicked()
        } // end MouseArea
    } // end Image back
} // end ToolBar root
