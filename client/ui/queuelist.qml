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

import model.Global 0.1
import model.QueueListModel 0.1

import "component"

Item
{
    id: root
    readonly property int fontsize: 15

    QueueListModel
    {
        id: queueListModel
        onDone:
        {
            comboBoxModel.clear();
            unlockUI()
            var list = queueListModel.result()
            if (list.length === 0)
            {
                if (queueListModel.hasError())
                {
                    status.text = queueListModel.lastError()
                }
                else
                {
                    status.text = "Done"
                }

                queueList.currentIndex = -1
                deleteBtn.enabled = false
                checkCreateRename()
                return;
            }

            status.text = "Done"
            var i = 0
            for (i = 0; i < list.length; ++i)
            {
                var toAppend = {}
                toAppend.text = list[i]
                comboBoxModel.append(toAppend)
            }

            queueList.currentIndex = 0
            deleteBtn.enabled = true;
            checkCreateRename()
        }
    }

    Component.onCompleted:
    {
        if (queueListModel.init())
        {
            Global.programExit(1, "Fail to initialize queue list model.")
        }
    }

    function lockUI()
    {
        refreshBtn.enabled = false
        deleteBtn.enabled = false
        createBtn.enabled = false
        renameBtn.enabled = false
        newName.enabled = false
    }

    function unlockUI()
    {
        refreshBtn.enabled = true
        deleteBtn.enabled = true
        createBtn.enabled = true
        renameBtn.enabled = true
        newName.enabled = true
    }

    function checkCreateRename()
    {
        var res = !(newName.text.length === 0)
        renameBtn.enabled = res
        createBtn.enabled = res;
    }

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
        enabled: false
        font.pointSize: fontsize
        text: "Delete"
        tooltip: "Delete current queue"

        anchors
        {
            top: parent.top
            right: refreshBtn.left
        }

        onClicked:
        {
            lockUI()
            queueListModel.startDelete(queueList.currentText)
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

        onClicked:
        {
            lockUI()
            queueListModel.startList()
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

        onTextEdited: checkCreateRename()
    }

    // row 3
    ToolTipButton
    {
        id: createBtn
        enabled: false
        font.pointSize: fontsize
        text: "Create"
        tooltip: "Create new queue"

        anchors
        {
            top: newNameText.bottom
            left: parent.left
        }

        onClicked:
        {
            lockUI()
            queueListModel.startCreate(newName.text)
        }
    }

    ToolTipButton
    {
        id: renameBtn
        enabled: false
        font.pointSize: fontsize
        text: "Rename"
        tooltip: "Rename selected queue"

        anchors
        {
            top: newNameText.bottom
            left: createBtn.right
        }

        onClicked:
        {
            lockUI()
            queueListModel.startRename(queueList.currentText, newName.text)
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
