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
import "component"

Item
{
    id: root

    property int fontSize: 15
    property bool accepted: false

    // row 1
    TitleText
    {
        id: hostText
        text: qsTr("Hosts: ")
        height: host.height

        anchors
        {
            top: parent.top
            left: parent.left
        }
    }

    ComboBox
    {
        id: host
        model: ListModel
        {
            id: comboBoxModel
        }

        anchors
        {
            top: parent.top
            left: hostText.right
            right: parent.right
        }
    }

    // row 2
    TitleText
    {
        id: aliasText
        text: qsTr("Alias: ")
        height: aliasName.height

        anchors
        {
            top: hostText.bottom
            left: parent.left
        }
    }

    TextInput
    {
        id: aliasName
        font.pointSize: fontSize
        maximumLength: 20

        anchors
        {
            top: host.bottom
            left: aliasText.right
            right: parent.right
        }
    }

    // row 3
    TitleText
    {
        id: ipText
        text: qsTr("IP: ")
        height: ip.height

        anchors
        {
            top: aliasText.bottom
            left: parent.left
        }
    }

    TextInput
    {
        id: ip
        font.pointSize: fontSize
        maximumLength: 20

        anchors
        {
            top: aliasName.bottom
            left: ipText.right
            right: parent.right
        }
    }

    // row 4
    TitleText
    {
        id: portText
        text: qsTr("Port: ")
        height: port.height

        anchors
        {
            top: ipText.bottom
            left: parent.left
        }
    }

    SpinBox
    {
        id: port
        font.pointSize: fontSize
        to: 65535
        editable: true

        anchors
        {
            top: ip.bottom
            left: portText.right
            right: parent.right
        }

        onValueModified:
        {
            if (port.value > 65535) port.value = 65535
            else if (port.value < 0) port.value = 0
        }
    }

    // row 5
    ToolTipButton
    {
        id: exitBtn
        tooltip: qsTr("Exit this page to Queue list page.")
        text: qsTr("Exit")
        font.pointSize: fontSize

        anchors
        {
            top: port.bottom
            right: parent.right
        }
    }

    ToolTipButton
    {
        id: deleteBtn
        tooltip: qsTr("Delete current profile")
        text: qsTr("Delete")
        font.pointSize: fontSize
        enabled: false

        anchors
        {
            top: port.bottom
            right: exitBtn.left
        }
    }

    ToolTipButton
    {
        id: saveBtn
        tooltip: qsTr("Save current profile")
        text: qsTr("Save")
        font.pointSize: fontSize
        enabled: false

        anchors
        {
            top: port.bottom
            right: deleteBtn.left
        }
    }

    ToolTipButton
    {
        id: connectBtn
        tooltip: qsTr("Connect to host")
        text: qsTr("Connect")
        font.pointSize: fontSize
        enabled: false

        anchors
        {
            top: port.bottom
            right: saveBtn.left
        }
    }

    // row 6
    TitleText
    {
        id: statusText
        text: qsTr("Status: ")

        anchors
        {
            top: connectBtn.bottom
            left: parent.left
        }
    }

    TitleText
    {
        id: status
        text: qsTr("Pending")

        anchors
        {
            top: connectBtn.bottom
            left: statusText.right
        }
    }
}
