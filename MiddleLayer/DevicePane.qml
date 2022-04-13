import QtQuick 2.5
import QtQuick.Window 2.1
import QtQuick.Controls 2.5

import QtQuick.Layouts 1.4

import "components"

GroupBox {
    property var model: null
    property alias deviceId : index.text

    id: root
    padding: 12
    width: parent.width
    background: Rectangle {
        color: "#73716F" //"#868482"
        border.color: "white"
        radius: 10
    }

    ColumnLayout {
        width: parent.width
        spacing: 5

        RowLayout {
            id: row
            spacing: 20
            width: parent.width
            anchors.margins: 15

            Row {
                spacing: 5
//              width: parent.width

                Text {
                    height: 25
                    width: 40
                    color: "white"
                    verticalAlignment: Text.AlignVCenter
                    text: "Device:"
                    font.pointSize: 10
                }

                CellEdit {
                    id: index
                }
            }

            Button {
                id: btnGet
                width: 80
                height: 28
                text: "Get"
                onClicked: {
                    model.requestDeviceInfo();
                }
            }

            Text {
                id: deviceName
                height: 25
                elide:  Text.ElideNone
                wrapMode: Text.Wrap
                maximumLineCount: 2
                color: "gold"
                verticalAlignment: Text.AlignVCenter
                text: model.deviceDescr === "" ? "The device info ..." : model.deviceDescr
                font.pointSize: 10
            }
        }

        RowLayout {
            spacing: 20
            width: parent.width
            anchors.margins: 5

            Row {
                spacing: 5
                Text {
                    height: 25
                    width: 40
                    color: "white"
                    verticalAlignment: Text.AlignVCenter
                    text: "Module:"
                    font.pointSize: 10
                }

                CellEdit {
                    id: edtValue
                }
            }

            Button {
                id: btnModuleGet
                width: 80
                height: 28
                text: "Get"
                onClicked: {
                    edtValue.focus = true
                }
            }

            Text {
                id: txtModuleName
                height: 25
                color: "gold"
                verticalAlignment: Text.AlignVCenter
                text: "The module info ... "
                font.pointSize: 10
            }

        }

    }
}

