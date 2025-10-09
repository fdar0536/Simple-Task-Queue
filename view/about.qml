import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 2.15

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
        anchors.left: layout.left
        anchors.right: layout.right
    }

    ColumnLayout
    {
        id: layout

        anchors.centerIn: parent

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

    Button
    {
        font.pointSize: 16
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
