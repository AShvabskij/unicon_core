import QtQuick 2.12
import QtQuick.Window 2.12
import QtQuick.Controls 2.3
import QtQuick.Controls.Material 2.0

import QtQuick.Layouts 1.1

import "components"
import solcon.qmlmodels 1.0

Window {
    id: main
    width: 520
    height: 770
    color: "#868482"

    visible: true
    property bool connected: false

    MainWindowVM {
        id: model
        deviceId: deviceNum.text
        paramIndex: paramIndex.text
        onDataReceived: {
            dataLog.text += "\n" + msg

        }
        onConnected: {
            main.connected = true;
        }

        onClosed: {
            main.connected = false;
        }
    }

    Column  {
        id: columnControls
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

        spacing: 20
        anchors.fill: parent
        anchors.margins: 10
        Row {
            spacing: 20
            width: parent.width

            Text {
                id: labelTitle
                width: parent.width
                horizontalAlignment: Text.AlignHCenter
                font.pixelSize: 30
                font.bold: true
                text: "UNICON SYSTEM"
            }
        }

        Divider {
            label: "Server"
        }

        Row {
            spacing: 20
            width: parent.width
            height: 30
            anchors.margins: 5

            Button {
                id: btnStart
                height: 30
                width: 120
                text: "Connect"
                onClicked: {
                    console.info("clicked! connected status = " + main.connected);

                    if (!main.connected) {
                        console.info("let's start");
                        model.start()
                    } else {
                        model.close()
                    }
                }
            }

            TextField {
                id: host
                width: 120
                height: 30
                color: "black"
                background: Rectangle {
                    border.color: "red"
                    color: "lightyellow"
                }
                text: qsTr("127.0.0.1:1235")
                font.pointSize: 10
            }

            Text {
                id: lblPort
                height: 30
                color: "#ffffff"
                verticalAlignment: Text.AlignVCenter
                lineHeightMode: Text.ProportionalHeight
                font.pointSize: 12
                fontSizeMode: Text.FixedSize
                text: main.connected ? "connected!" : "closed"
            }
        }

        Divider {
            label: "Devices"
        }

        Row {
            id: rowDevice

            spacing: 20
            width: parent.width
            anchors.margins: 5

            Button {
                id: btnDevice
                width: 120
                text: "Device"
                onClicked: {
                    model.receiveDeviceInfo()
                }
            }

            TextField {
                id: deviceNum
                width: 120
                height: 30
                color: "black"
                background: Rectangle {
                    border.color: "red"
                    color: "lightyellow"
                }
                text: qsTr("0")
                font.pointSize: 12
            }

            Text {
                id: deviceName
                height: 30
                color: "#ffffff"
                verticalAlignment: Text.AlignVCenter
                text: model.deviceName
                font.pointSize: 12
            }
        }

        Divider {
            label: "Params"
        }

        Row {
            id: rowParam
            spacing: 20
            width: parent.width
            anchors.margins: 5
            Button {
                id: btnParamInfo
                width: 120
                text: "Parameter"
                onClicked: {
                    model.receiveParamInfo();
                }
            }

            TextField {
                id: paramIndex
                width: 120
                height: 30
                color: "black"
                background: Rectangle {
                    border.color: "red"
                    color: "lightyellow"
                }
                text: qsTr("0")
                font.pointSize: 12
            }

            Text {
                id: paramName
                height: 30
                color: "white"
                verticalAlignment: Text.AlignVCenter
                text: model.paramName
                font.pointSize: 12
            }
        }

        Row {
            id : rowParamValues
            spacing: 20

            width: parent.width
            anchors.margins: 5
            Button {
                id: btnParamValues
                width: 120
                text: "Get values"
                onClicked: {
                    model.receiveParamValues()
                }
            }

            TextField {
                id: valueParamIndex
                width: 120
                height: 30
                color: "black"
                background: Rectangle {
                    border.color: "red"
                    color: "lightyellow"
                }
                text: qsTr("0")
                font.pointSize: 12
            }

            Text {
                id: paramValue
                height: 30
                color: "white"
                verticalAlignment: Text.AlignVCenter
                text: model.paramValue
                font.pointSize: 12
            }
        }

        Divider {
            label: "Data Log"
        }

        Row {
            spacing: 20
            width: parent.width
            anchors.margins: 5

            TextArea {
                id: dataLog
                height: 300
                width: parent.width
                color : "white"
                font.pointSize: 10
                background: Rectangle {
                    border.color: "black"
                    border.width: 2
                    radius: 4
                    color: "#868482"
                }

//              anchors.fill: parent
                text: qsTr("Hello\nHello")
            }
        }
    }

}
