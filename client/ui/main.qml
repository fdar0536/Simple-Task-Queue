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
import QtQuick.Dialogs
import QtQuick.Controls
import QtQuick.Controls.Material 2.12

import model.Global 0.1
import model.MainModel 0.1
import "component"

ApplicationWindow
{
    id: root
    title: qsTr("Simple Task Queue")

    property int currentPage: 0
    readonly property var pages: [
                                      "qrc:/ui/settings.qml",
                                      "qrc:/ui/queuelist.qml"
                                 ]

    visible: true
    minimumWidth: 700
    minimumHeight: 500

    MainModel
    {
        id: mainModel

        onShowWindow:
        {
            root.show()
        }
    }

    function getIsInPortrait()
    {
        if (root.width <= 750) return true
        else if (root.width < root.height) return true
        else return false
    }

    onClosing: function(close)
    {
        close.accepted = false;
        root.hide()
    }

    onAfterSynchronizing:
    {
        if (mainModel.init())
        {
            Global.programExit(1, "Fail to initialize main model.")
        }
    }

    MainToolBar
    {
        id: toolBar

        onMenuClicked:
        {
            if (!getIsInPortrait()) return

            if (mainMenu.opened)
            {
                mainMenu.close()
            }
            else
            {
                mainMenu.open()
            }
        }

        onAboutQtClicked: mainModel.aboutQt()

        onExitClicked: mainModel.programExit()
    }

    MainMenu
    {
        id: mainMenu
        inPortrait: getIsInPortrait()
        menuWidth:
        {
            if (root.width > 200) return 150
            else root.width * 0.8
        }

        menuHeight: root.height - toolBar.height
        y: toolBar.height

        onIndexChanged: function(index)
        {
            if (Global.isSettingsNotAccepted())
            {
                if (index !== 0)
                {
                    return
                }

                if (currentPage === 0)
                {
                    return
                }

                currentPage = 0
                loader.source = pages[0]
            }

            currentPage = index
            loader.source = pages[index]
        } // end onIndexChanged
    }

    Flickable
    {
        id: flickable

        anchors.fill: parent
        anchors.topMargin: toolBar.height
        anchors.leftMargin: !getIsInPortrait() ? mainMenu.width : undefined

        Loader
        {
            id: loader
            anchors.fill: parent
            source: "qrc:/ui/settings.qml"
        }
    }
}