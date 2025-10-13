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

Item
{
    Tumbler
    {
        id: tumbler
        anchors.horizontalCenter: rowLayout.horizontalCenter
        anchors.bottomMargin: 5
        anchors.bottom: rowLayout.top
    } // Tumbler

    RowLayout
    {
        id: rowLayout
        anchors.centerIn: parent

        MyButton
        {
            text: qsTr("Connect")
        }

        MyButton
        {
            text: qsTr("Clear")
        }

        MyButton
        {
            text: qsTr("Load")
        }

        MyButton
        {
            text: qsTr("Save")
        }
    } // RowLayout

    ColumnLayout
    {
        anchors.topMargin: 10
        anchors.top: rowLayout.bottom
        anchors.horizontalCenter: rowLayout.horizontalCenter
        width: rowLayout.width
        spacing: 10

        MyTextField
        {
            placeholderText: qsTr("Name")
            Layout.fillWidth: true
        }

        MyTextField
        {
            placeholderText: qsTr("Host")
            Layout.fillWidth: true
        }

        RowLayout
        {
            Layout.fillWidth: true
            Label
            {
                font.pointSize: 16
                text: qsTr("Port: ")
            }

            SpinBox
            {
                font.pointSize: 16
                from: 0
                to: 65535
                editable: true
                value: 12345
            }
        }
    } // ColumnLayout
} // Item
