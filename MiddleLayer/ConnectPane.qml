import QtQuick 2.5
import QtQuick.Window 2.1
import QtQuick.Controls 2.5

import QtQuick.Layouts 1.4

import "components"

Item {
    property bool connected: false
    property var model: null

    width: parent.width
    height: 30

    Row {
        id: rowConnect

        spacing: 20
        width: parent.width
        height: parent.height
        anchors.margins: 5

        Rectangle {
            height: 1
            width: 12
            opacity: 0
        }

        Button {
            id: btnStart
            height: 30
            width: 120
            text: connected ? "Disconnect" : "Connect"
            onClicked: {
//              console.info("clicked! connected status = " + connected);

                if (!connected) {
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
                color: "lightgray"
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
            text: main.connected ? "connected!" : ""
        }
    }
}
