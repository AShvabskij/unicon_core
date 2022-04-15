import QtQuick 2.1
import QtQuick.Window 2.1
import QtQuick.Controls 2.3

import QtQuick.Layouts 1.1
import QtQuick.Controls.Material 2.12
import QtQuick.Dialogs 1.1

import "components"
import solcon.qmlmodels 1.0

ApplicationWindow {
    id: main
    width: 750
    height: 860
    color: "gray"//"#868482"

    visible: true
    property bool connected: false

    Component.onCompleted: {
        x = Screen.width / 2 - width / 2
        y = Screen.height / 2 - height / 2
    }

    title: "unicon"

    MainWindowVM {
        id: mainModel
        deviceId: devicePane.deviceId
        paramId1: readParam.paramId
        paramId2: writeParam.paramId

        onDataReceived: {
            dataLog.text += "Received: " + msg + "\n"
            flickable.contentY = (flickable.contentHeight - flickable.height) > 0 ? flickable.contentHeight - flickable.height : flickable.contentY
        }
        onDataRequest: {
            dataLog.text += "\nSend: " + msg + "\n"
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
                text: "UNICON TEST PANEL"
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
                PropertyChanges { target: connectPane; connected: true }
                PropertyChanges { target: devicePane; enabled: true }
                PropertyChanges { target: oscPane; enabled: true }
                PropertyChanges { target: paramsPane; enabled: true }

            },
            State {
                name: "disconnected"
                PropertyChanges { target: connectPane; connected: false }
                PropertyChanges { target: devicePane; enabled: false }
                PropertyChanges { target: oscPane; enabled: false }
                PropertyChanges { target: paramsPane; enabled: false }
            }
        ]

        state: main.connected ? "connected" : "disconnected"

        spacing: 10
//      anchors.fill: parent
        width: parent.width
        anchors.top: parent.top
        anchors.margins: 10

        ConnectPane {
            id: connectPane
            model : mainModel
        }

        Column {
            id: devices
            spacing: 1
            width: parent.width

            Divider {
                label: "Devices"
            }

            DevicePane {
                id: devicePane
                model : mainModel
            }
            OscPane {
                id: oscPane
                model : mainModel
            }
        }

        Column {
            id: paramsPane
            spacing: 1
            width: parent.width

            Divider {
                label: "Params"
            }

            ParameterPane {
                id: readParam
                writable: false
                model: mainModel
                info: mainModel.paramInfo1
                value: mainModel.paramValue1
                valueInfo: mainModel.paramValueInfo1
            }
            ParameterPane {
                id: writeParam
                writable: true
                model: mainModel
                info: mainModel.paramInfo2
                value: mainModel.paramValue2
                valueInfo: mainModel.paramValueInfo2
            }
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

    ToolBar {
        id : tbRow
        anchors.top: columnControls.bottom
        height: 30
        background: Rectangle {
            border.color: "gray"
            border.width: 1
            color: "black" //"#868482"
        }

        width: parent.width

        Row {
            anchors.leftMargin: 10
            anchors.fill: parent
            width: parent.width
            spacing: 1

            Button {
                id: btnLogClear
                anchors.verticalCenter: parent.verticalCenter
                width: 60
                height: parent.height - 10
                text: "clear"
                onClicked: {
                    dataLog.clear()
                }
            }

            Button {
                id: btnLogSave
                anchors.verticalCenter: parent.verticalCenter
                width: 60
                height: parent.height - 10
                text: "save"
            }

            Button {
                id: btnLogDisable
                anchors.verticalCenter: parent.verticalCenter
                width: 60
                height: parent.height - 10
                text: "select all"
            }


            Item {
                Layout.fillWidth: true
            }

        }
    }

    Flickable {
        id: flickable
        anchors.top: tbRow.bottom
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
                color: "black"
            }
            placeholderText: qsTr("Log send/receive data here...")
            selectByMouse : true
            persistentSelection: true
        }
        ScrollBar.vertical: ScrollBar { id: scroll}
    }
}


