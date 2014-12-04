import QtQuick 2.1
import QtQuick.Controls 1.2
import QtQuick.Controls.Styles 1.2
import "../colors.js" as Colors

ButtonStyle {
    id: style
    background: Rectangle {
        implicitWidth: 26
        implicitHeight: 26
        border.width: 1
        border.color: control.pressed ?  Colors.lightOrange : Colors.brightGray
        radius: 5
        gradient: Gradient {
            GradientStop {color: control.pressed ? "#000000" : "#222222"; position: 0.0}
            GradientStop {color: control.pressed ? "#161106" : "#111111"; position: 0.3}
            GradientStop {color: control.pressed ? "#161106" : "#111111"; position: 0.7}
            GradientStop {color: control.pressed ? "#272217" : "#000000"; position: 1.0}
        }
    }
    label: Text {
        color: control.pressed ? Colors.whiteOrange : "white"
        anchors.centerIn: parent
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        text: control.text
    }
}
