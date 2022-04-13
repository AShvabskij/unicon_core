import QtQuick 2.5
import QtQuick.Window 2.1
import QtQuick.Controls 2.5

import QtQuick.Layouts 1.4

import "components"

GroupBox {
    property var model: null
    property alias oscId: txtOscId.text
    property alias chNum: txtChannelNum.text
    property bool oscStreaming: false

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
            id: osc
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
                    text: "Osc:"
                    font.pointSize: 10
                }

                CellEdit {
                    id: txtOscId
                }
            }

            Button {
                id: btnOscGet
                width: 80
                height: 28
                text: "Get"
                onClicked: {
                    if (!oscStreaming) {
                        model.requestOscInfo(root.oscId)
                    }
                }
            }

            Text {
                id: txtOscInfo
                height: 25
                color: "yellow"
                verticalAlignment: Text.AlignVCenter
                text: model.oscDescr
                font.pointSize: 10
            }
        }

        RowLayout {
            id: channel
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
                    text: "Channel:"
                    font.pointSize: 10
                }

                CellEdit {
                    id: txtChannelNum
                    width: 80
                    height: 25
                }
            }

            Button {
                id: btnChannelGet
                width: 80
                height: 28
                text: "Get"
                onClicked: {
                    if (oscStreaming) return;

                    model.requestChannelInfo(root.oscId, root.chNum)
                }
            }

            Button {
                id: btnChannelStart
                width: 80
                height: 28
                text: oscStreaming ? "Stop" : "Start"
                onClicked: {
                    if (oscStreaming) {
                        model.stopOscParamValues(root.oscId)
                        oscStreaming = false
                    } else {
                        model.startOscParamValues(root.oscId, root.chNum)
                        oscStreaming = true
                    }
                }
            }

            Text {
                id: txtChannelInfo
                height: 25
                color: "yellow"
                verticalAlignment: Text.AlignVCenter
                text: model.oscChannelValue
                font.pointSize: 10
            }
        }

    }
}
