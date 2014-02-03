import QtQuick 2.1
import QtQuick.Controls 1.1
import QtQuick.Controls.Styles 1.1

Item {
    id: surround
    property alias text: button.text
    property alias action: button.action
    width: 30
    height: 30
    Button {
        id: button
        width: 26
        height: 26
        x: 2
        y: 2
        style: ButtonStyle {
            background: Rectangle {
                implicitWidth: 26
                implicitHeight: 26
                border.width: 2
                border.color: "#A0A0A0"
                radius: 5
                color: control.pressed ? "#202020" : "#000000"
            }
            label: Text {
                color: "white"
                anchors.centerIn: parent
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                text: control.text
            }
        }
    }
}

