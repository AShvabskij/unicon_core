import QtQuick 2.5
import QtQuick.Controls 2.5

TextField {
    width: 80
    height: 25
    color: "black"
    selectByMouse: true
    placeholderText: qsTr("0")
    background: Rectangle {
        border.color: "red"
        color: "lightgray"
    }
    text: qsTr("")
    font.pointSize: 10
}
