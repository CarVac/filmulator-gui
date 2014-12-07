import QtQuick 2.2
import QtQuick.Controls 1.2
import QtQuick.Controls.Styles 1.2
import QtQuick.Layouts 1.1
import QtQuick.Dialogs 1.2
import "gui_components"
import "colors.js" as Colors

Rectangle {
    id: root
    property real uiScale: 1
    color: Colors.darkGrayL
    anchors.fill: parent

    signal tooltipWanted(string text, int x, int y)

    ColumnLayout {
        id: settingsList
        spacing: 0 * uiScale
        x: 3 * uiScale
        y: 3 * uiScale
        width: 300 * uiScale

        ToolSlider {
            id: uiScaleSlider
            title: qsTr("User Interface Scale")
            tooltipText: qsTr("This is a multiplier for the size of all controls in the program.")
            minimumValue: 0.5
            maximumValue: 4.0
            stepSize: 0.1
            defaultValue: settings.getUiScale()
            changed: false
            onValueChanged: {
                if (value == defaultValue) {
                    uiScaleSlider.changed = false
                } else {
                    uiScaleSlider.changed = true
                }
            }
            Component.onCompleted: {
                uiScaleSlider.tooltipWanted.connect(root.tooltipWanted)
            }
        }
        ToolButton {
            id: saveSettings
            text: qsTr("Save Settings")
            tooltipText: qsTr("Apply settings and save for future use")
            width: settingsList.width
            height: 40 * uiScale
            action: Action {
                onTriggered: {
                    settings.uiScale = uiScaleSlider.value
                    uiScaleSlider.defaultValue = uiScaleSlider.value
                    uiScaleSlider.changed = false
                }
            }
        }
    }
}
