/*
 * Simple Task Queue
 * Copyright (c) 2025-present fdar0536
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
import QtQuick.Layouts

import "components"

import ff.backend.main 1.0
import ff.backend.global 1.0

ApplicationWindow
{
    id: app
    width: 1280
    height: 720
    visible: true
    title: "Flex Flow GUI"

    Main
    {
        id: main
        onShow:
        {
            app.visible = true;
        }
    }

    Component.onCompleted:
    {
        if (main.init())
        {
            main.exit();
        }
    }

    onClosing: (close) =>
    {
        close.accepted = false;
        app.visible = false;
    }

    header: ColumnLayout
    {
        TabBar
        {
            Layout.fillWidth: true

            MyTabButton
            {
                text: qsTr("Settings")
                onClicked:
                {
                    loader.source = "qrc:/qt/qml/FF/view/settings.qml";
                }
            }

            MyTabButton
            {
                text: qsTr("Queue list")
            }

            MyTabButton
            {
                text: qsTr("Queue")
            }

            MyTabButton
            {
                text: qsTr("Output")
            }

            MyTabButton
            {
                text: qsTr("About")
                onClicked:
                {
                    loader.source = "qrc:/qt/qml/FF/view/about.qml";
                }
            }

            MyTabButton
            {
                text: qsTr("Exit")
                onClicked:
                {
                    main.exitProcess(true);
                }
            }
        } // TabBar

        MyText
        {
            id: status
            Layout.alignment: Qt.AlignHCenter
            text: qsTr("Not connected")
        }
    } // header: ColumnLayout



    Loader
    {
        id: loader
        anchors.fill: parent
        source: "qrc:/qt/qml/FF/view/settings.qml"
    }
} // ApplicationWindow
