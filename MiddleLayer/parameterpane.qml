import QtQuick 2.5
import QtQuick.Window 2.1
import QtQuick.Controls 2.5

import QtQuick.Layouts 1.4
import QtQuick.Controls.Styles 1.4
import QtQuick.Extras 1.4

import "components"

GroupBox {
    id: root
    property var model: null
    property alias info: paramInfo.text
    property alias value: edtValue.text
    property bool writable: false
    property bool streaming: false
    property bool oscStreaming: false
    property int num: 0
    property string paramId: moduleIndex.text + "." + paramIndex.text

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
            id: rowParam
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
                    text: "Param:"
                    font.pointSize: 10
                }

                CellEdit {
                    id: moduleIndex
                    width: 60
                    placeholderText: qsTr("Mod id")
                }
                CellEdit {
                    id: paramIndex
                    width: 60
                    placeholderText: qsTr("Par id")
                }
            }

            Button {
                id: btnParamGet
                width: 80
                height: 28
                text: "Get"
                onClicked: {
                    model.requestParamInfo(num, root.paramId);
                    model.requestParamValues(root.paramId);
                }
            }

            Text {
                id: paramInfo
                height: 25
                color: "gold"
                verticalAlignment: Text.AlignVCenter
                text: "The param info"
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
                    text: "Value:"
                    font.pointSize: 10
                }

                CellEdit {
                    id: edtValue
                    width: 125
                    readOnly: true
                }

            }

            Button {
                id: btnParamEdit
                width: 80
                height: 28
                text: "Set"
                onClicked: {
                    model.changeParamValue(num, root.paramId, edtValue.text);
                }
            }

            Button {
                id: btnTrend
                width: 120
                height: 28
                text: streaming ? "Trend off" : "Trend on"
                onClicked: {
                    if (streaming) {
                        model.stopStreamParamValues(root.paramId)
                        streaming = false
                    } else {
                        model.startStreamParamValues(num, root.paramId)
                        streaming = true
                    }
                }
            }

            /*
            SpinBox {
                id: edt_spinbox
                height: 28
                width: 60
                value: 1
                font.pointSize:10
                up.indicator.implicitHeight: 10
                down.indicator.implicitHeight: 10
                inputMethodHints : Qt.ImhFormattedNumbersOnly
                visible: false
            }

            Button {
                id: edt_btnOnOff
                width: 80
                height: 28
                text: "On"
                onClicked: {
                    text=  "Off"
                }
                visible: false
            }
*/

        }

    }
}

