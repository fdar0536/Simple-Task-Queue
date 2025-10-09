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

    ColumnLayout
    {
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

        Button
        {
            font.pointSize: 16
            text: qsTr("About Qt")
            onClicked:
            {
                about.aboutQt();
            }
        }
    }
}
