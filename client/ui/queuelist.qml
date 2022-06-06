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

import QtQml
import QtQuick 2.15
import QtQuick.Controls
import QtQuick.Controls.Material 2.12

import "component"

Item
{
    id: root
    readonly property int fontsize: 15

    // row 1
    TitleText
    {
        id: queueListText
        height: deleteBtn.height
        text: "Queue list: "

        anchors
        {
            top: parent.top
            left: parent.left
        }
    }

    ComboBox
    {
        id: queueList
        font.pointSize: fontsize
        height: deleteBtn.height
        model: ListModel
        {
            id: comboBoxModel
        }

        anchors
        {
            left: queueListText.right
            top: parent.top
            right: deleteBtn.left
        }
    }

    ToolTipButton
    {
        id: deleteBtn
        font.pointSize: fontsize
        text: "Delete"
        tooltip: "Delete current queue"

        anchors
        {
            top: parent.top
            right: refreshBtn.left
        }
    }

    ToolTipButton
    {
        id: refreshBtn
        font.pointSize: fontsize
        text: "Refresh"
        tooltip: "Refresh queue list"

        anchors
        {
            top: parent.top
            right: parent.right
        }
    }

    // row 2
    TitleText
    {
        id: newNameText
        text: "New name: "
        font.pointSize: fontsize
        height: newName.height

        anchors
        {
            top: queueListText.bottom
            left: parent.left
        }
    }

    TextInput
    {
        id: newName
        font.pointSize: fontsize

        anchors
        {
            top: queueListText.bottom
            left: newNameText.right
            right: parent.right
        }
    }

    // row 3
    ToolTipButton
    {
        id: createBtn
        font.pointSize: fontsize
        text: "Create"
        tooltip: "Create new queue"

        anchors
        {
            top: newNameText.bottom
            left: parent.left
        }
    }

    ToolTipButton
    {
        id: renameBtn
        font.pointSize: fontsize
        text: "Rename"
        tooltip: "Rename selected queue"

        anchors
        {
            top: newNameText.bottom
            left: createBtn.right
        }
    }

    // row 4
    TitleText
    {
        id: statusText
        font.pointSize: fontsize
        text: "Status: "

        anchors
        {
            top: createBtn.bottom
            left: parent.left
        }
    }

    TitleText
    {
        id: status
        font.pointSize: fontsize
        text: "Pending"

        anchors
        {
            top: createBtn.bottom
            left: statusText.right
        }
    }
}
