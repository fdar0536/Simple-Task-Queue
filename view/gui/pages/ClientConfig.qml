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
    id: root

    CTRL.RemoteClient
    {
        id: ctrl
    }

    // functions
    function onCtrlInitDone()
    {
        busyIndicator.running = false;
        updateRemoteList();
        updatePrevNextBtn();
        if (!Global.isNotMobile)
        {
            // is mobile
            useRemote.checkState = Qt.Checked;
            useRemote.enabled = false;
            remoteConfig.enabled = true;
            return;
        }

        if (Global.isLocalAvailable)
        {
            if (Global.backendMode === 0)
            {
                // grpc
                useRemote.checkState = Qt.Checked;
            }
            else
            {
                // local
                useRemote.checkState = Qt.Unchecked;
            }

            useRemote.enabled = true;
            remoteConfig.enabled = false;
            return;
        }
    }

    function updateRemoteInfo()
    {
        remoteName.text = ctrl.name;
        remoteHost.text = ctrl.ip;
        remotePort.value = ctrl.port;
    }

    function updateRemoteList()
    {
        updateRemoteInfo();
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
        deleteBtn.enabled = true;
    }

    function updatePrevNextBtn()
    {
        prevBtn.enabled = ctrl.hasPrevPage;
        nextBtn.enabled = ctrl.hasNextPage;
    }

    function onCtrlServerConnectDone(res)
    {
        busyIndicator.running = false;
        root.enabled = true;
        if (res)
        {
            msgDialog.info(qsTr("Done"),
                           qsTr("Connect to server successfully"));
        }
        else
        {
            msgDialog.error(qsTr("Fail to connect to Server"));
        }
    }

    MsgDialog
    {
        id: msgDialog
    }

    Component.onCompleted:
    {
        ctrl.InitDone.connect(onCtrlInitDone);
        ctrl.ServerConnectDone.connect(onCtrlServerConnectDone);
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
            var isRemote = (useRemote.checkState === Qt.Checked);
            remoteConfig.enabled = isRemote;

            if (isRemote)
            {
                Global.backendMode = 0; // grpc
            }
            else
            {
                Global.backendMode = 1; // local
            }
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

                    onCurrentIndexChanged:
                    {
                        if (currentIndex === -1) return;
                        var res = ctrl.setLastDataIndex(currentIndex);
                        updateRemoteInfo();
                        if (!res)
                        {
                            msgDialog.error(qsTr("Fail to set last data index"));
                        }
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
                    id: prevBtn
                    text: qsTr("Prev")
                    toolTip: qsTr("Previous page of profile")
                    enabled: false

                    onClicked:
                    {
                        var res = ctrl.prevPage();
                        updatePrevNextBtn();
                        if (!res)
                        {
                            msgDialog.error(qsTr("Fail to switch to previous page"));
                            return;
                        }

                        updateRemoteList();
                    }
                }

                ToolTipButton
                {
                    id: nextBtn
                    text: qsTr("Next")
                    toolTip: qsTr("Next page of profile")
                    enabled: false

                    onClicked:
                    {
                        var res = ctrl.nextPage();
                        updatePrevNextBtn();
                        if (!res)
                        {
                            msgDialog.error(qsTr("Fail to switch to next page"));
                            return;
                        }

                        updateRemoteList();
                    }
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
                            return;
                        }

                        ctrl.updateData();
                        updateRemoteList();
                        updatePrevNextBtn();
                    }
                }

                ToolTipButton
                {
                    id: deleteBtn
                    text: qsTr("Delete")
                    toolTip: qsTr("Delete current profile")
                    enabled: false

                    onClicked:
                    {
                        deleteBtn.enabled = false;
                        ctrl.deleteData(remoteList.currentText);
                        ctrl.updateData();
                        updateRemoteList();
                        updatePrevNextBtn();
                    }
                }

                Button
                {
                    text: qsTr("Connect")
                    onClicked:
                    {
                        if (!ctrl.startConnect(remoteHost.text, remotePort.value))
                        {
                            msgDialog.error(qsTr("Fail to switch to next page"));
                            return;
                        }

                        root.enabled = false;
                        busyIndicator.running = true;
                    }
                }
            } // end Row
        } // end colunm
    } // end GroupBox clientConfig
} // end Page
