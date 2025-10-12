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

import ff.backend.about 1.0


Item
{
    id: root

    About
    {
        id: about
    }

    Image
    {
        source: "qrc:/qt/qml/FF/original-icon.jpg"
        width: layout.width
        height: layout.width

        anchors.bottomMargin: 5
        anchors.bottom: layout.top
        anchors.horizontalCenter: layout.horizontalCenter
    }

    ColumnLayout
    {
        id: layout

        anchors.centerIn: parent

        MyText
        {
            text: "Flex Flow"
        }

        MyText
        {
            text:
            {
                return "Version: " + about.version;
            }
        }

        MyText
        {
            text:
            {
                return "Branch: " + about.branch;
            }
        }

        MyText
        {
            text:
            {
                return "Commit: " + about.commit;
            }
        }
    } // ColumnLayout

    MyButton
    {
        text: qsTr("About Qt")
        width: layout.width

        anchors.topMargin: 5
        anchors.top: layout.bottom
        anchors.left: layout.left
        anchors.right: layout.right

        onClicked:
        {
            about.aboutQt();
        }
    }
}
