import QtQuick 2.3
import QtQuick.Controls 1.2
import QtQuick.Controls.Styles 1.2
import "../colors.js" as Colors

RadioButtonStyle {
    id: style
    property real uiScale: 1
    background: Rectangle {
        id: backgroundRect
        implicitWidth: 26 * uiScale
        implicitHeight: 26 * uiScale
        border.width: 1 * uiScale
        border.color: control.checked ? Colors.lightOrange : Colors.brightGray
        radius: 5 * uiScale
        gradient: Gradient {
            GradientStop {color: control.checked ? "#000000" : "#222222"; position: 0.0}
            GradientStop {color: control.checked ? "#161106" : "#111111"; position: 0.3}
            GradientStop {color: control.checked ? "#161106" : "#111111"; position: 0.7}
            GradientStop {color: control.checked ? "#272217" : "#000000"; position: 1.0}
        }
    }
    label: Item {
        width: control.width
        height: control.height
        Text {
            color: control.checked ? Colors.whiteOrange : Colors.brightGrayH//"white"
            anchors.centerIn: parent
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            text: control.text
            font.pixelSize: 12.0 * uiScale
        }
    }
    indicator: Item{}
}
