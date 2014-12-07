import QtQuick 2.1
import QtQuick.Controls 1.2
import QtQuick.Controls.Styles 1.2
import "../colors.js" as Colors

ButtonStyle {
    id: style
    property real uiScale: 1
    property bool notDisabled: true
    background: Rectangle {
        implicitWidth: 26 * uiScale
        implicitHeight: 26 * uiScale
        border.width: 1 * uiScale
        border.color: control.pressed ?  Colors.lightOrange : Colors.brightGray
        radius: 5 * uiScale
        gradient: Gradient {
            GradientStop {color: control.pressed ? "#000000" : "#222222"; position: 0.0}
            GradientStop {color: control.pressed ? "#161106" : "#111111"; position: 0.3}
            GradientStop {color: control.pressed ? "#161106" : "#111111"; position: 0.7}
            GradientStop {color: control.pressed ? "#272217" : "#000000"; position: 1.0}
        }
    }
    label: Text {
        color: notDisabled ? (control.pressed ? Colors.whiteOrange : "white") : Colors.middleGray
        anchors.centerIn: parent
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        text: control.text
        font.pointSize: 9.0 * uiScale
    }
}
