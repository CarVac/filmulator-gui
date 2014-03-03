import QtQuick 2.0
import QtQuick.Controls 1.1
import QtQuick.Controls.Styles 1.1

Rectangle {
    id: root
    implicitHeight: 30
    implicitWidth: parent.width
    property alias text: checkBox.text
    property alias tooltipText: tooltip.tooltipText
    property alias checked: checkBox.checked
    property bool defaultChecked

    property real __padding: 2

    signal tooltipWanted(string text, int coordX, int coordY)

    color: "#303030"

    CheckBox {
        id: checkBox
        anchors.verticalCenter: parent.verticalCenter
        x: __padding
        text: "Check Box"
        style: CheckBoxStyle {
            indicator: Rectangle {
                implicitWidth: 20
                implicitHeight: 20
                radius: 3
                border.color: control.checked ? "#A87848" : "#808080"
                border.width: 1
                color: control.checked ? "#FF9922" : "#B0B0B0"
                Rectangle {
                    visible: control.checked
                    color: "#555"
                    border.color: "#333"
                    radius: 1
                    anchors.margins: 4
                    anchors.fill: parent
                }
            }
            label: Text{
                color: "white"
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                text: control.text
            }
        }
    }
    ToolTip {
        id: tooltip
        anchors.fill: checkBox
        Component.onCompleted: {
            tooltip.tooltipWanted.connect(root.tooltipWanted)
        }
    }
}
