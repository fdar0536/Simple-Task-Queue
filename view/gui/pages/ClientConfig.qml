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

import Global
import ClientConfig as CTRL
import "../components"

Page
{
    CTRL.ClientConfig
    {
        id: ctrl
    }

    function onCtrlInitDone()
    {
        busyIndicator.running = false;
        if (!Global.isNotMobile)
        {
            useRemote.checkState = Qt.Checked;
            useRemote.enabled = false;
            remoteConfig.enabled = true;
            return;
        }

        if (Global.isLocalAvailable)
        {
            useRemote.checkState = Qt.Unchecked;
            useRemote.enabled = true;
            remoteConfig.enabled = false;
            return;
        }

        if (ctrl.rowCount() <= 0)
        {
            remoteList.currentIndex = -1;
            return;
        }

        remoteList.currentIndex = 0;
    }

    MsgDialog
    {
        id: msgDialog
    }

    Component.onCompleted:
    {
        ctrl.InitDone.connect(onCtrlInitDone);
        if (!ctrl.init())
        {
            msgDialog.error(qsTr("Fail to initialize"));
        }
    }

    BusyIndicator
    {
        id: busyIndicator
        anchors.centerIn: parent
        width: 50
        height: 50
        z: 50
        running: true
    }

    TitleText
    {
        id: clientTitle
        text: qsTr("Client Configuration")
    }

    Row
    {
        id: rowTop
        spacing: 5
        anchors.top: clientTitle.bottom
        TitleText
        {
            text: qsTr("Log Level: ")
            anchors.verticalCenter: logLevel.verticalCenter
        }

        ComboBox
        {
            id: logLevel
            model: [
                "Trace",
                "Debug",
                "Info",
                "Warning",
                "Error",
                "Critical",
                "Off"]

            currentIndex: 2
            onCurrentIndexChanged:
            {
                var newIndex = ctrl.setLogLevel(currentIndex);
                if (newIndex === currentIndex)
                {
                    return;
                }

                logLevel.currentIndex = newIndex;
            }
        }
    }

    CheckBox
    {
        id: useRemote
        text: qsTr("Use GRPC client")
        font.pointSize: 12
        anchors.top: rowTop.bottom
        enabled: false
        onClicked:
        {
            remoteConfig.enabled =
                    (useRemote.checkState === Qt.Checked);
        }
    }

    GroupBox
    {
        id: remoteConfig
        enabled: false
        anchors.top: useRemote.bottom

        title: qsTr("GRPC Client config")
        font.pointSize: 12

        Column
        {
            spacing: 5

            ComboBox
            {
                id: remoteList
                model: ctrl
                currentIndex: -1
                delegate: ItemDelegate
                {
                    text: ctrl.name(index)
                }

                onCurrentIndexChanged:
                {
                    if (ctrl.rowCount() === 0)
                    {
                        return;
                    }

                    remoteName.text = ctrl.name(currentIndex);
                    remoteHost.text = ctrl.ip(currentIndex);
                    remotePort.value = ctrl.port(currentIndex);
                }
            }

            TextField
            {
                id: remoteName
                placeholderText: "Name"
            }

            TextField
            {
                id: remoteHost
                placeholderText: "Host"
            }

            Row
            {
                spacing: 5

                TitleText
                {
                    text: qsTr("Port: ")
                    anchors.verticalCenter: remotePort.verticalCenter
                }

                SpinBox
                {
                    id: remotePort
                    from: 0
                    to: 65535
                    value: 12345
                    editable: true
                }
            }

            Row
            {
                Button
                {
                    text: qsTr("Clear")
                    onClicked:
                    {
                        remoteName.text = "";
                        remoteHost.text = "";
                        remotePort.value = 12345;
                    }
                }

                Button
                {
                    text: qsTr("Save")
                }

                Button
                {
                    text: qsTr("Connect")
                }
            } // end Row
        } // end colunm
    } // end GroupBox clientConfig
} // end Page
