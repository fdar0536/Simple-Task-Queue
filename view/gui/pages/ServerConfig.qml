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
import QtQuick.Dialogs
import QtQuick.Controls
import QtQuick.Controls.Material

import Global
import ServerConfig as CTRL
import "../components"

Page
{
    CTRL.ServerConfig
    {
        id: ctrl
    }

    MsgDialog
    {
        id: msgDialog
    }

    function onCtrlInitDone()
    {
        busyIndicator.running = false
        if (Global.isNotMobile)
        {
            autoStartServer.enabled = true;
            if (ctrl.autoStartServer)
            {
                autoStartServer.checkState = Qt.Checked;
            }
            else
            {
                autoStartServer.checkState = Qt.Unchecked;
            }

            if (!Global.isLocalAvailable)
            {
                return;
            }

            enableServer.enabled = true;

            if (ctrl.isServerRunning)
            {
                enableServer.checkState = Qt.Checked;
                serverConfig.enabled = true;
                startBtn.enabled = false;
                stopBtn.enabled = true;

                serverIp.text = ctrl.serverIP();
                serverPort.value = ctrl.serverPort();
            }
            else
            {
                enableServer.checkState = Qt.Unchecked;
                serverConfig.enabled = false;
                startBtn.enabled = true;
                stopBtn.enabled = false;
            }
        }
    } // end function onCtrlInitDone()

    Component.onCompleted:
    {
        ctrl.InitDone.connect(onCtrlInitDone);
        ctrl.init();
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
        id: serverTitle
        text: qsTr("Server Configuration")
    }

    CheckBox
    {
        id: autoStartServer
        anchors.top: serverTitle.bottom
        text: qsTr("Auto start server")
        font.pointSize: 12
        enabled: false
        onClicked:
        {
            ctrl.autoStartServer =
                    (autoStartServer.checkState === Qt.Checked);
        }
    }

    CheckBox
    {
        id: enableServer
        anchors.top: autoStartServer.bottom
        text: qsTr("Enable server")
        font.pointSize: 12
        enabled: false
        onClicked:
        {
            var res = (enableServer.checkState === Qt.Checked);
            serverConfig.enabled =res
            if (!res)
            {
                ctrl.stopServer();
                startBtn.enabled = true;
                stopBtn.enabled = false;
            }
        }
    }

    GroupBox
    {
        id: serverConfig
        enabled: false
        anchors.top: enableServer.bottom

        title: qsTr("server config")
        font.pointSize: 12

        Column
        {
            spacing: 5
            TextField
            {
                id: serverIp
                placeholderText: "IP"
            }

            Row
            {
                spacing: 5

                TitleText
                {
                    text: qsTr("Port: ")
                    anchors.verticalCenter: serverPort.verticalCenter
                }

                SpinBox
                {
                    id: serverPort
                    from: 0
                    to: 65535
                    value: 12345
                    editable: true
                }
            }

            Row
            {
                spacing: 5
                Button
                {
                    id: startBtn
                    enabled: true
                    text: qsTr("Start")
                    onClicked:
                    {
                        if (!ctrl.setServerIP(serverIp.text))
                        {
                            msgDialog.error(qsTr("Invalid ip"));
                            return;
                        }

                        ctrl.setServerPort(serverPort.value);

                        if (!ctrl.startServer())
                        {
                            msgDialog.error(qsTr("Fail to start server"));
                            return;
                        }

                        startBtn.enabled = false;
                        stopBtn.enabled = true;
                    }
                }

                Button
                {
                    id: stopBtn
                    enabled: false
                    text: qsTr("Stop")
                    onClicked:
                    {
                        ctrl.stopServer();
                        startBtn.enabled = true;
                        stopBtn.enabled = false;
                    }
                }
            } // end Row
        } // end Colunm
    } // end GroupBox serverConfig
} // end Page
