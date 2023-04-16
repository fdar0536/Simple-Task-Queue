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
import QtQuick.Dialogs

import Global
import "../components"

Item
{
    id: root

    signal goBackClicked()

    function addLog(log)
    {
        textArea.append(log);
    }

    function onWindowClosing()
    {
        root.goBackClicked();
    }

    Component.onCompleted: function()
    {
        Global.WindowClosing.connect(onWindowClosing);
    }

    FileDialog
    {
        id: fileDialog
        fileMode: FileDialog.SaveFile
        onAccepted: function()
        {
            // https://stackoverflow.com/questions/24927850/get-the-path-from-a-qml-url
            var fileName = fileDialog.selectedFile.toString();
            fileName = fileName.replace(/^(file:\/{3})/,"");
            fileName = decodeURIComponent(fileName);

            Global.saveFile(fileName, textArea.text);
        }
    }

    ScrollView
    {
        id: scrollView
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: saveBtn.top

        TextArea
        {
            id: textArea
            readOnly: true
        }
    }

    ToolTipButton
    {
        id: saveBtn
        enabled: Global.isNotMobile

        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: clearBtn.top

        text: qsTr("Save")
        toolTip: qsTr("Save the log");
        onClicked: function()
        {
            fileDialog.open();
        }
    }

    ToolTipButton
    {
        id: clearBtn

        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: backBtn.top

        text: qsTr("Clear")
        toolTip: qsTr("Clear the log")
        onClicked: textArea.clear()
    }

    ToolTipButton
    {
        id: backBtn

        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom

        text: qsTr("Go back")
        toolTip: qsTr("Go Back")
        onClicked: root.goBackClicked()
    }
}
