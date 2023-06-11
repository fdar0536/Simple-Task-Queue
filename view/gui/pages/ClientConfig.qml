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
import RemoteClient as CTRL
import "../components"

Page
{
    CTRL.RemoteClient
    {
        id: ctrl
    }

    function onCtrlInitDone()
    {
        busyIndicator.running = false;
        updateRemoteList();
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
    }

    function updateRemoteList()
    {
        remoteName.text = ctrl.name;
        remoteHost.text = ctrl.ip;
        remotePort.value = ctrl.port;

        var data = ctrl.data();
        remoteListModel.clear();
        if (data.length <= 0)
        {
            remoteList.currentIndex = -1;
            return;
        }

        for (let i = 0; i < data.length; ++i)
        {
            remoteListModel.append(data[i]);
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

            Row
            {
                TitleText
                {
                    text: qsTr("Profile: ")
                    anchors.verticalCenter: remoteList.verticalCenter
                }

                ComboBox
                {
                    id: remoteList
                    currentIndex: -1

                    model: ListModel
                    {
                        id: remoteListModel
                    }
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
                ToolTipButton
                {
                    text: qsTr("Prev")
                    toolTip: qsTr("Previous page of profile")
                    enabled: false
                }

                ToolTipButton
                {
                    text: qsTr("Next")
                    toolTip: qsTr("Next page of profile")
                    enabled: false
                }

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
                    onClicked:
                    {
                        if (!ctrl.saveSetting(remoteName.text,
                                              remoteHost.text,
                                              remotePort.value))
                        {
                            msgDialog.error(qsTr("Invalid ip or port"));
                        }

                        ctrl.updateData();
                        updateRemoteList();
                    }
                }

                Button
                {
                    text: qsTr("Connect")
                }
            } // end Row
        } // end colunm
    } // end GroupBox clientConfig
} // end Page
