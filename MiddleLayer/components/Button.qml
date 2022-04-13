import QtQuick 2.0
import QtGraphicalEffects 1.1

Item {
    id: button
    width: 80
    height: 20
    property alias text: innerText.text
    signal clicked
    layer.effect: DropShadow {
           verticalOffset: 1
           color: button.visualFocus ? "#330066ff" : "#aaaaaa"
           spread: 0.5
       }
    Image {
        id: backgroundImage
        anchors.fill: parent
        source: (button.enabled ? "images/button_background_normal.png" : "images/button_background_disabled.png")
    }

    Text {
        id: innerText
        anchors.centerIn: parent
        color: "white"
        font.pointSize: 10
        font.bold: false
    }

    //Mouse area to react on click events
    MouseArea {
        anchors.fill: button
        onClicked: { button.clicked();}
        onPressed: {
            backgroundImage.source = "images/button_background_pressed.png" }
        onReleased: {
            backgroundImage.source = (button.enabled ? "images/button_background_normal.png" : "images/button_background_disabled.png")
        }
    }
}
