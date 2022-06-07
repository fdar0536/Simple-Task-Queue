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

Drawer
{
    required property bool inPortrait
    property alias menuWidth: root.width
    property alias menuHeight: root.height
    signal indexChanged(int index)

    id: root

    modal: inPortrait
    interactive: inPortrait
    position: inPortrait ? 0 : 1
    visible: !inPortrait

    ListModel
    {
        id: listModel
        ListElement
        {
            name: 'Settings'
        }

        ListElement
        {
            name: 'Queue list'
        }
    }

    ListView
    {
        id: listView
        anchors.fill: parent
        model: listModel
        delegate: Rectangle
        {
            width: parent.width
            height: 25
            color: 'white'
            Text
            {
                id: itemText
                text: name
                font.pointSize: 15
            }

            MouseArea
            {
                anchors.fill: parent
                hoverEnabled: true
                onClicked:
                {
                    listView.currentIndex = index
                    root.indexChanged(index)
                }

                onEntered:
                {
                    parent.color = 'gray'
                    itemText.color = 'white'
                }

                onExited:
                {
                    parent.color = 'white'
                    itemText.color = 'black'
                }
            }
        }
    } // end ListView listView
} // end Drawer root
