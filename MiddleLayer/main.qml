import QtQuick 2.1
import QtQuick.Window 2.1
import QtQuick.Controls 2.3

import QtQuick.Layouts 1.1

import "components"
import solcon.qmlmodels 1.0

Window {
    id: main
    width: 720
    height: 770
    color: "#868482"

    visible: true
    property bool connected: false

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

        spacing: 20
//      anchors.fill: parent
        width: parent.width
        anchors.top: parent.top


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
            id: rowConnect

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

            Text {
                height: 30
                color: "#ffffff"
                verticalAlignment: Text.AlignVCenter
                text: "Device header, id:"
                font.pointSize: 12
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

            Button {
                id: btnDevice
                width: 120
                text: "Get"
                onClicked: {
                    model.receiveDeviceInfo()
                }
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

            Text {
                height: 30
                color: "white"
                verticalAlignment: Text.AlignVCenter
                text: "Param header, id:"
                font.pointSize: 12
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

            Button {
                id: btnParamInfo
                width: 120
                text: "Get"
                onClicked: {
                    model.receiveParamInfo();
                }
            }
        }

        Row {
            id : rowParamValues
            spacing: 20

            width: parent.width
            anchors.margins: 5

            Text {
                height: 30
                color: "white"
                verticalAlignment: Text.AlignVCenter
                text: "Param value, id:  "
                font.pointSize: 12
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

            Button {
                id: btnParamValues
                width: 120
                text: "Get"
                onClicked: {
                    model.receiveParamValues()
                }
            }

            Button {
                id: btnParamStream
                width: 120
                text: "Stream"
                onClicked: {
                    model.streamParamValues()
                }
            }

        }

        Divider {
            label: "Data Log"
        }
    } // column


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
            color : "white"
            font.pointSize: 12
            font.bold: true

            leftPadding: 6
            rightPadding: 6
            topPadding: 6
            bottomPadding: 6

            background: Rectangle {
                border.color: "black"
                border.width: 2
                radius: 4
                color: "#868482"
            }

            text: qsTr("Hello\nHello")
        }
        ScrollBar.vertical: ScrollBar { id: scroll}
    }
}


