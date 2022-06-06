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
import model.SettingsModel 0.1
import "component"

Item
{
    id: root

    property int fontSize: 15
    property bool accepted: false
    property bool dirty: false
    property var settings: ({})
    property var configs: []
    property bool ipAccepted: false

    Component.onCompleted:
    {
        if (settingsModel.init())
        {
            Global.programExit(1, "Fail to initialize settings' model.")
        }

        settings = Global.settings()
        configs = settings["config"]
        accepted = false
        dirty = false

        var lastState = Global.state("settingsState")
        if (typeof lastState === 'string')
        {
            if (configs.length === 0)
            {
                host.currentIndex = -1
                return
            }

            updateComboBox()
            host.currentIndex = 0
            updateUI(0)
            ipAccepted = true
        }
        else
        {
            updateComboBox()
            host.currentIndex = lastState["hostIndex"]
            aliasName.text = lastState["alias"]
            ip.text = lastState["ip"]
            port.value = lastState["port"]
        }

        connectBtn.enabled = true
        saveBtn.enabled = true
        deleteBtn.enabled = true
    }

    Component.onDestruction:
    {
        if (root.dirty)
        {
            settings["config"] = configs
            Global.saveSettings(root.settings)
        }

        if (accepted)
        {
            var lastState = {}
            lastState["hostIndex"] = host.currentIndex
            lastState["alias"] = aliasName.text
            lastState["ip"] = ip.text
            lastState["port"] = port.value
            Global.setState("settingsState", lastState)
        }
    }

    function checkAllInput()
    {
        return (ipAccepted && (aliasName.text.length !== 0))
    }

    function updateComboBox()
    {
        var i = 0
        var config = {}
        comboBoxModel.clear()
        for (i = 0; i < configs.length; ++i)
        {
            config = root.configs[i]
            var toAppend = {}
            toAppend.text = config["alias"]
            comboBoxModel.append(toAppend)
        }
    }

    function updateUI(index)
    {
        var config = root.configs[index]
        host.currentIndex = index
        aliasName.text = config["alias"]
        ip.text = config["ip"]
        port.value = config["port"]
    }

    function lockUI()
    {
        aliasName.enabled = false
        ip.enabled = false
        port.enabled = false
    }

    function unlockUI()
    {
        aliasName.enabled = true
        ip.enabled = true
        port.enabled = true
    }

    SettingsModel
    {
        id: settingsModel
        onDone:
        {
            if (settingsModel.hasError())
            {
                status.text = settingsModel.lastError()
                unlockUI()
                return
            }

            accepted = true
            status.text = "Connected"
            unlockUI()
        }
    }

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

        onCurrentIndexChanged:
        {
            if (currentIndex === -1) return

            var config = root.configs[host.currentIndex]
            aliasName.text = config["alias"]
            ip.text = config["ip"]
            port.value = config["port"]
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

        onTextEdited:
        {
            accepted = false
            var res = checkAllInput()
            connectBtn.enabled = res
            saveBtn.enabled = res
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

        validator: RegularExpressionValidator
        {
            regularExpression: /^(?:(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.){3}(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)$/
        }

        onTextEdited:
        {
            accepted = false
            root.ipAccepted = ip.acceptableInput
            var res = checkAllInput()
            connectBtn.enabled = res
            saveBtn.enabled = res
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
        id: deleteBtn
        tooltip: qsTr("Delete current profile")
        text: qsTr("Delete")
        font.pointSize: fontSize
        enabled: false

        anchors
        {
            top: port.bottom
            right: parent.right
        }

        onClicked:
        {
            configs.splice(host.currentIndex, 1)
            dirty = true
            if (configs.length === 0)
            {
                deleteBtn.enabled = false
            }

            updateComboBox()
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

        onClicked:
        {
            var res = {}
            res["alias"] = aliasName.text
            res["ip"] = ip.text
            res["port"] = port.value

            configs.push(res)
            updateComboBox()
            updateUI(configs.length - 1)
            dirty = true
            if (!deleteBtn.enabled)
            {
                deleteBtn.enabled = true
            }
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

        onClicked:
        {
            lockUI()
            accepted = false
            status.text = "Connecting. May wait up to 1.5 minutes."
            settingsModel.startConnect(ip.text, port.value)
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
