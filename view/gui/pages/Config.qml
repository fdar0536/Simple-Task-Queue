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
import Config as CTRL

import "../components"

Item {
    id: root

    CTRL.Config
    {
        id: ctrl
    }

    function init()
    {
        if (Global.isNotMobile && ctrl.isLocalAvailable)
        {
            useRemote.enabled = true;
            useRemote.checkState = Qt.Unchecked;
            remoteConfig.enabled = false;
        }
        else
        {
            useRemote.enabled = false;
            useRemote.checkState = Qt.Checked;
            remoteConfig.enabled = true;
        }

        if (Global.isNotMobile)
        {
            enableServer.enabled = true;

            if (ctrl.isServerRunning)
            {
                enableServer.checkState = Qt.Checked;
                serverConfig.enabled = true;
            }
            else
            {
                enableServer.checkState = Qt.Unchecked;
                serverConfig.enabled = false;
            }
        }
    }

    Component.onCompleted:
    {
        init();
    }

    SwipeView
    {
        id: swipeView
        anchors.fill: parent

        Page
        {
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
                        "Critical"]

                    currentIndex: 2
                    onCurrentIndexChanged:
                    {
                    }
                }
            }

            Row
            {
                id: row1
                spacing: 5
                anchors.top: rowTop.bottom
                CheckBox
                {
                    id: useRemote
                    text: qsTr("Use GRPC client")
                    font.pointSize: 12
                    onClicked:
                    {
                        remoteConfig.enabled =
                                (useRemote.checkState === Qt.Checked);
                    }
                }

                Button
                {
                    text: qsTr("Refresh")
                    onClicked:
                    {
                        init();
                    }
                }
            }

            GroupBox
            {
                id: remoteConfig
                anchors.top: row1.bottom

                title: qsTr("GRPC Client config")
                font.pointSize: 12

                Column
                {
                    spacing: 5

                    ComboBox
                    {
                        id: remoteList
                        model: ListModel
                        {
                            id: remoteListModel
                        }

                        onCurrentIndexChanged:
                        {
                            if (remoteListModel.count === 0)
                            {
                                return;
                            }

                            remoteName.text = model.text;
                            remoteHost.text = model.host;
                            remotePort.value = model.port;
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

        Page
        {
            TitleText
            {
                id: serverTitle
                text: qsTr("Server Configuration")
            }

            CheckBox
            {
                id: enableServer
                anchors.top: serverTitle.bottom
                text: qsTr("Enable server")
                font.pointSize: 12
                enabled: Global.isNotMobile
                onClicked:
                {
                    serverConfig.enabled =
                            (enableServer.checkState === Qt.Checked);
                }
            }

            GroupBox
            {
                id: serverConfig
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
                            text: qsTr("Start")
                        }

                        Button
                        {
                            text: qsTr("Stop")
                        }
                    }
                } // end Colunm
            } // end GroupBox serverConfig
        } // end Page
    } // end SwipeView swipeView

    PageIndicator
    {
        count: swipeView.count
        currentIndex: swipeView.currentIndex

        anchors.bottom: parent.bottom
        anchors.horizontalCenter: parent.horizontalCenter
    }
} // end Item root
