import QtQuick 2.15
import QtQuick.Controls 2.15
import ff.backend.main 1.0

ApplicationWindow
{
    id: app
    width: 1280
    height: 720
    visible: true

    Main
    {
        id: main
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
        main.exitProcess(true);
    }

    header: TabBar
    {
        TabButton
        {
            text: qsTr("Settings")
        }

        TabButton
        {
            text: qsTr("Queue list")
        }

        TabButton
        {
            text: qsTr("Queue")
        }

        TabButton
        {
            text: qsTr("Output")
        }

        TabButton
        {
            text: qsTr("Log")
        }

        TabButton
        {
            text: qsTr("About")
        }

        TabButton
        {
            text: qsTr("Exit")
            onClicked:
            {
                main.exitProcess(true);
            }
        }
    }
}
