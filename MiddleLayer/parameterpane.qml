import QtQuick 2.5
import QtQuick.Window 2.1
import QtQuick.Controls 2.5

import QtQuick.Layouts 1.4
import QtQuick.Controls.Styles 1.4
import QtQuick.Extras 1.4

import "components"

GroupBox {
    id: pane
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

                TextField {
                    id: paramIndex
                    width: 80
                    height: 25
                    color: "black"
                    background: Rectangle {
                        border.color: "red"
                        color: "lightgray"
                    }
                    text: qsTr("0")
                    font.pointSize: 10
                }
            }

            Button {
                id: btnParamGet
                width: 80
                height: 28
                text: "Get"
                onClicked: {
                    //          model.receiveParamInfo();
                }
            }

            Text {
                id: paramName
                height: 25
                color: "white"
                verticalAlignment: Text.AlignVCenter
                text: "The param name"
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

                TextField {
                    id: edtValue
                    width: 80
                    height: 25
                    color: "black"
                    background: Rectangle {
                        border.color: "red"
                        color: "lightgray"
                    }
                    text: qsTr("0")
                    font.pointSize: 10
                }
            }

            Button {
                id: btnParamEdit
                width: 80
                height: 28
                text: "Edit"
                onClicked: {
                    edtValue.focus = true
                }
            }

            Button {
                id: btnTrend
                width: 120
                height: 28
                text: "Trend"
                onClicked: {}
            }

            Button {
                id: btnOsc
                width: 120
                height: 28
                text: "Oscilloscope"
                onClicked: {}
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

