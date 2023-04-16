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

import QtQuick.Controls
import QtQuick.Dialogs
import Global
import MainCtrl
import "components"

ApplicationWindow
{
    id: root
    visible: true
    property bool isNotInit: true
    width: 1280
    height: 720
    header: menuBar

    MainCtrl
    {
        id: ctrl
    } // end Main

    MessageDialog
    {
        id: msgDialog
    }

    MenuBar
    {
        id: menuBar
    }

    MainMenu
    {
        id: mainMenu
        width: Math.min(root.width, root.height) / 3 * 2
        height: root.height
    }

    function onCtrlShow()
    {
        root.visible = true;
    } // end onCtrlShow

    function onCtrlExit()
    {
        onCtrlShow();
        msgDialog.buttons = MessageDialog.Ok | MessageDialog.No;
        msgDialog.title = qsTr("Exit");
        msgDialog.text = qsTr("Are you sure to exit?");
        msgDialog.accepted.connect(() => { Qt.exit(0) })
        msgDialog.open();
    }

    onClosing: function(close)
    {
        close.accepted = false;
        if (Global.isNotMobile)
        {
            root.visible = false;
        }
        else
        {
            Global.notifyClosing();
        }
    }

    onAfterSynchronizing: function()
    {
        if (isNotInit)
        {
            Qt.font.pointSize = 25;
            if (!ctrl.init())
            {
                msgDialog.buttons = MessageDialog.Ok;
                msgDialog.title = qsTr("Error");
                msgDialog.text = qsTr("Fail to initialize");
                msgDialog.accepted.connect(() => { Qt.exit(1) })
                msgDialog.open();
            }

            isNotInit = false;

            ctrl.Show.connect(onCtrlShow);
            ctrl.Exit.connect(onCtrlExit);

            // menuBar
            menuBar.menuClicked.connect(mainMenu.open)
            menuBar.infoClicked.connect(ctrl.AboutQt);
            menuBar.closeClicked.connect(onCtrlExit);
            Global.AllCleaned.connect(onCtrlExit);
        }
    } // end onAfterSynchronizing
} // end ApplicationWindow root
