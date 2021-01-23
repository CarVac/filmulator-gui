import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import "gui_components"
import "colors.js" as Colors

Rectangle {
    id: root
    property real uiScale: 1
    color: Colors.darkGrayL
    Layout.fillWidth: true
    Layout.fillHeight: true

    signal tooltipWanted(string text, int x, int y)

    ColumnLayout {
        id: settingsList
        spacing: 0 * uiScale
        x: 3 * uiScale
        y: 3 * uiScale
        width: 350 * uiScale

        ToolSlider {
            id: uiScaleSlider
            title: qsTr("User Interface Scale")
            tooltipText: qsTr("This is a multiplier for the size of all controls in the program.\n\nThis setting takes effect after applying settings and then restarting Filmulator.")
            minimumValue: 0.5
            maximumValue: 4.0
            stepSize: 0.1
            tickmarksEnabled: true
            tickmarkFactor: 5
            minorTicksEnabled: true
            value: settings.getUiScale()
            defaultValue: settings.getUiScale()
            valueText: value.toFixed(1)
            changed: false
            onValueChanged: {
                if (Math.abs(value - defaultValue) < 0.05) {
                    uiScaleSlider.changed = false
                } else {
                    uiScaleSlider.changed = true
                }
            }
            Component.onCompleted: {
                uiScaleSlider.tooltipWanted.connect(root.tooltipWanted)
            }
            uiScale: root.uiScale
        }

        ToolSwitch {
            id: useSystemLanguageSwitch
            text: qsTr("Use system language")
            tooltipText: true ? qsTr("Turning this off will set the language to English.\n\nThis setting takes effect after applying settings and then restarting Filmulator.") : qsTr("Turning this off will let you select the interface language from a list.\n\nThis setting takes effect after applying settings and then restarting Filmulator.")
            isOn: settings.getUseSystemLanguage()
            defaultOn: settings.getUseSystemLanguage()
            onIsOnChanged: useSystemLanguageSwitch.changed = true
            Component.onCompleted: {
                useSystemLanguageSwitch.tooltipWanted.connect(root.tooltipWanted)
                useSystemLanguageSwitch.changed = false
            }
            uiScale: root.uiScale
        }

        ToolSwitch {
            id: mipmapSwitch
            text: qsTr("Smooth editor image")
            tooltipText: qsTr("This enables mipmaps for the Filmulate tab's image view. It's recommended for noisy images where not mipmapping may cause patterns to appear at different zoom levels.\n\nIt has slight impact on responsiveness for the last few tools, but it doesn't affect performance when zooming and panning. It also softens the image slightly, which may be undesireable.\n\nThis is applied as soon as you save settings.")
            isOn: settings.getMipmapView()
            defaultOn: settings.getMipmapView()
            onIsOnChanged: mipmapSwitch.changed = true
            Component.onCompleted: {
                mipmapSwitch.tooltipWanted.connect(root.tooltipWanted)
                mipmapSwitch.changed = false
            }
            uiScale: root.uiScale
        }

        ToolSwitch {
            id: lowMemModeSwitch
            text: qsTr("Reduce memory usage")
            tooltipText: qsTr("Warning: VERY SLOW!\n\nEnabling this turns off high-resolution caching in the editor. It will consume less memory but the full resolution image will recompute from the beginning for any edit you make.\n\nThis setting takes effect after applying settings and then restarting Filmulator.")
            isOn: settings.getLowMemMode()
            defaultOn: settings.getLowMemMode()
            onIsOnChanged: lowMemModeSwitch.changed = true
            Component.onCompleted: {
                lowMemModeSwitch.tooltipWanted.connect(root.tooltipWanted)
                lowMemModeSwitch.changed = false
            }
            uiScale: root.uiScale
        }

        ToolSwitch {
            id: quickPreviewSwitch
            text: qsTr("Render small preview first")
            tooltipText: qsTr("Enabling this causes the editor to process a small-size image before processing at full resolution, for better responsiveness. It will make it take longer before you can export an image, though.\n\nThis takes effect after applying settings and restarting Filmulator.")
            isOn: settings.getQuickPreview()
            defaultOn: settings.getQuickPreview()
            onIsOnChanged: quickPreviewSwitch.changed = true
            Component.onCompleted: {
                quickPreviewSwitch.tooltipWanted.connect(root.tooltipWanted)
                quickPreviewSwitch.changed = false
            }
            uiScale: root.uiScale
        }

        ToolSlider {
            id: previewResSlider
            title: qsTr("Preview render resolution")
            tooltipText: qsTr("When the small preview is active, the preview image will be processed at an image size with this value as the long dimension. The larger this is, the sharper the preview, but the longer it takes to generate.\n\nThis takes effect after applying settings and restarting Filmulator.")
            minimumValue: 100
            maximumValue: 8000
            stepSize: 100
            value: settings.getPreviewResolution()
            defaultValue: settings.getPreviewResolution()
            changed: false
            onValueChanged: {
                if (Math.abs(value - defaultValue) < 0.5) {
                    previewResSlider.changed = false
                } else {
                    previewResSlider.changed = true
                }
            }
            Component.onCompleted: {
                previewResSlider.tooltipWanted.connect(root.tooltipWanted)
            }
            uiScale: root.uiScale
        }

        ToolButton {
            id: saveSettings
            text: qsTr("Save Settings")
            tooltipText: qsTr("Apply settings and save for future use")
            width: settingsList.width
            height: 40 * uiScale
            notDisabled: uiScaleSlider.changed || useSystemLanguageSwitch.changed || mipmapSwitch.changed || lowMemModeSwitch.changed || quickPreviewSwitch.changed || previewResSlider.changed
            onTriggered: {
                settings.uiScale = uiScaleSlider.value
                uiScaleSlider.defaultValue = uiScaleSlider.value
                uiScaleSlider.changed = false
                settings.useSystemLanguage = useSystemLanguageSwitch.isOn
                useSystemLanguageSwitch.defaultOn = useSystemLanguageSwitch.isOn
                useSystemLanguageSwitch.changed = false
                settings.mipmapView = mipmapSwitch.isOn
                mipmapSwitch.defaultOn = mipmapSwitch.isOn
                mipmapSwitch.changed = false
                settings.lowMemMode = lowMemModeSwitch.isOn
                lowMemModeSwitch.defaultOn = lowMemModeSwitch.isOn
                lowMemModeSwitch.changed = false
                settings.quickPreview = quickPreviewSwitch.isOn
                quickPreviewSwitch.defaultOn = quickPreviewSwitch.isOn
                quickPreviewSwitch.changed = false
                settings.previewResolution = previewResSlider.value
                previewResSlider.defaultValue = previewResSlider.value
                previewResSlider.changed = false
            }
            uiScale: root.uiScale
        }
    }
    ColumnLayout {
        id: dataUpdateList
        spacing: 0 * uiScale
        x: 358 * uiScale
        y: 3 * uiScale
        width: 350 * uiScale

        Rectangle {
            id: lensfunCheck
            width: parent.width
            height: 45 * uiScale
            property real padding: 4 * uiScale

            color: Colors.darkGray

            Text {
                id: checkLabel
                color: "white"
                width: parent.width - checkButton.width - 2*parent.padding
                x: parent.padding
                y: parent.padding
                font.pixelSize: 12.0 * uiScale
                text: qsTr("Check for lens correction updates")
            }

            Rectangle {
                id: checkResultBox
                width: parent.width - checkButton.width - 2*parent.padding
                height: 20 * uiScale
                x: parent.padding
                y: 20*uiScale + parent.padding
                color: "black"

                Text {
                    id: checkResult
                    color: "white"
                    x: lensfunCheck.padding / 2
                    y: 1 * uiScale
                    width: parent.width - x
                    height: parent.height - y
                    font.pixelSize: 12.0 * uiScale
                    text: settings.lensfunStatus
                }
            }

            ToolButton {
                id: checkButton
                width: 100 * uiScale
                height: 45 * uiScale
                anchors.right: parent.right
                y: 0 * uiScale
                text: qsTr("Check","Check for lensfun updates")
                onTriggered: {
                    settings.checkLensfunStatus()
                }

                uiScale: root.uiScale
            }
        }
        Rectangle {
            id: lensfunUpdate
            width: parent.width
            height: 45 * uiScale
            property real padding: 4 * uiScale

            color: Colors.darkGray

            Text {
                id: updateLabel
                color: "white"
                width: parent.width - updateButton.width - 2*parent.padding
                x: parent.padding
                y: parent.padding
                font.pixelSize: 12.0 * uiScale
                text: qsTr("Update lens correction database")
            }

            Rectangle {
                id: updateBox
                width: parent.width - updateButton.width - 2*parent.padding
                height: 20 * uiScale
                x: parent.padding
                y: 20*uiScale + parent.padding
                color: "black"

                Text {
                    id: updateResult
                    color: "white"
                    x: lensfunCheck.padding / 2
                    y: 1 * uiScale
                    width: parent.width - x
                    height: parent.height - y
                    font.pixelSize: 12.0 * uiScale
                    text: settings.updateStatus
                }
            }

            ToolButton {
                id: updateButton
                width: 100 * uiScale
                height: 45 * uiScale
                anchors.right: parent.right
                y: 0 * uiScale
                text: qsTr("Update","Update lensfun database")
                onTriggered: {
                    settings.updateLensfun()
                }

                uiScale: root.uiScale
            }
        }
        Rectangle {
            id: camconstSpacer
            width: parent.width
            height: 4 * uiScale
            color: Colors.darkGray
            opacity: 0
        }
        Rectangle {
            id: camconstDownload
            width: parent.width
            height: 45 * uiScale
            property real padding: 4 * uiScale

            color: Colors.darkGray

            Text {
                id: camconstDownloadLabel
                color: "white"
                width: parent.width - downloadButton.width - 2*parent.padding
                x: parent.padding
                y: parent.padding
                font.pixelSize: 12.0 * uiScale
                text: qsTr("Download latest camera constants")
            }
            Rectangle {
                id: camconstDownloadBox
                width: parent.width - downloadButton.width - 2*parent.padding
                height: 20 * uiScale
                x: parent.padding
                y: 20*uiScale + parent.padding
                color: "black"

                Text {
                    id: camconstDownloadResult
                    color: "white"
                    x: lensfunCheck.padding / 2
                    y: 1 * uiScale
                    width: parent.width - x
                    height: parent.height - y
                    font.pixelSize: 12.0 * uiScale
                    text: settings.camconstDlStatus
                }
            }
            ToolButton {
                id: downloadButton
                width: 100 * uiScale
                height: 45 * uiScale
                anchors.right: parent.right
                y: 0 * uiScale
                text: qsTr("Download","Download new camconst.json")
                onTriggered: {
                    settings.downloadCamConst()
                }

                uiScale: root.uiScale
            }
        }
    }
    Text {
        id: versionText
        color: "white"
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.margins: 4 * uiScale
        font.pixelSize: 12.0 * uiScale
        text: "v0.11.0" + " "
    }
}
