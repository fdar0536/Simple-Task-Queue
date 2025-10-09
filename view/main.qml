import QtQuick 2.15
import QtQuick.Controls 2.15

import "components"

import ff.backend.main 1.0

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

    header: TabBar
    {
        MyTabButton
        {
            text: qsTr("Settings")
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
        }

        MyTabButton
        {
            text: qsTr("Exit")
            onClicked:
            {
                main.exitProcess(true);
            }
        }
    } // header: TabBar

    Loader
    {
        id: loader
        anchors.fill: parent
        source: "qrc:/qt/qml/FF/view/about.qml"
    }
} // ApplicationWindow
