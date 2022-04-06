import QtQuick 2.1
import QtQuick.Window 2.1
import QtQuick.Controls 2.3

import QtQuick.Layouts 1.1
import QtQuick.Controls.Material 2.12

import "components"
import solcon.qmlmodels 1.0

ApplicationWindow {
    id: main
    width: 720
    height: 770
    color: "gray"//"#868482"

    visible: true
    property bool connected: false

    Component.onCompleted: {
        x = Screen.width / 2 - width / 2
        y = Screen.height / 2 - height / 2
    }

    MainWindowVM {
        id: model
        deviceId: deviceNum.text
        paramIndex: paramIndex.text
        valueParamIndex: valueParamIndex.text

        onDataReceived: {
            dataLog.text += "\n" + msg
            flickable.contentY = (flickable.contentHeight - flickable.height) > 0 ? flickable.contentHeight - flickable.height : flickable.contentY
        }
        onConnected: {
            main.connected = true;
        }

        onClosed: {
            main.connected = false;
        }
    }

    Action {
        id: mainMenu
        icon.name: "back"
        onTriggered: {
        }
    }

    header: ToolBar {
        Material.foreground: "white"

        RowLayout {
            spacing: 20
            anchors.fill: parent
/*
            ToolButton {
                action: mainMenu
            }
*/
            Label {
                id: titleLabel
                text: "UNICON SYSTEM"
                font.pixelSize: 20
                elide: Label.ElideRight
                horizontalAlignment: Qt.AlignHCenter
                verticalAlignment: Qt.AlignVCenter
                Layout.fillWidth: true
            }
        }
    }

    Column  {
        id: columnControls

//      leftPadding: 12

        states: [
            State {
                name: "connected"
                PropertyChanges { target: btnStart; text: "Close" }
                PropertyChanges { target: rowDevice; enabled: true }
                PropertyChanges { target: rowParam; enabled: true }
                PropertyChanges { target: rowParamValues; enabled: true }

            },
            State {
                name: "disconnected"
                PropertyChanges { target: btnStart; text: "Connect" }
                PropertyChanges { target: rowDevice; enabled: false }
                PropertyChanges { target: rowParam; enabled: false }
                PropertyChanges { target: rowParamValues; enabled: false }
            }
        ]

        state: main.connected ? "connected" : "disconnected"

        spacing: 10
//      anchors.fill: parent
        width: parent.width
        anchors.top: parent.top
        anchors.margins: 10

        ConnectPane{}

        Divider {
            label: "Devices"
        }

        DevicePane {

        }

        Divider {
            id: test
            label: "Params"
        }

        Column {
            spacing: 1
            width: parent.width

            ParameterPane {}
            ParameterPane {}
        }

        Divider {
            label: "Data Log"
        }

    } // column


/*
    Item {
        anchors.top: buttonRow.bottom
        ListView {
            id: listview
            anchors.top: columnControls.bottom
            height: 200

            width: parent.width
            model : 2
            boundsBehavior: Flickable.StopAtBounds
            delegate: ParameterPane {}

            contentWidth: 520
            flickableDirection: Flickable.AutoFlickDirection
        }
        Scrollbar {
            flickableItem: list
            align: Qt.AlignTrailing
        }
*/
    Flickable {
        id: flickable
        anchors.top: columnControls.bottom
        anchors.topMargin: 10
        height: parent.height - columnControls.height - 30
        width: parent.width
        flickableDirection: Flickable.VerticalFlick

        TextArea.flickable: TextArea {

            id: dataLog
            width: parent.width
            wrapMode: TextArea.Wrap
            color : "white"
            font.pointSize: 10
            font.bold: false

            leftPadding: 6
            rightPadding: 6
            topPadding: 6
            bottomPadding: 6

            background: Rectangle {
                border.color: "black"
                border.width: 2
                radius: 4
                color: "black" //"#868482"
            }
            text: qsTr("Hello\nHello")
            placeholderText: qsTr("Enter description")
            selectByMouse : true
        }
        ScrollBar.vertical: ScrollBar { id: scroll}
    }

}


