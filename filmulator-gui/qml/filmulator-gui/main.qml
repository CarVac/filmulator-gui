import QtQuick 2.2
import QtQuick.Controls 1.2
import QtQuick.Layouts 1.1
import QtQuick.Dialogs 1.1
import QtQuick.Controls.Styles 1.2
import "gui_components"

ApplicationWindow {
    id: root
    title: qsTr("Filmulator")
    property int tempVisibility
    width: 1024
    height: 768
    minimumHeight: 600
    minimumWidth:800

    signal tooltipWanted(string text, int x, int y)

    Rectangle {
        id: fillRect
        anchors.fill: parent
        color: "#FF303030"
    }

    SplitView {
        anchors.fill: parent
        orientation: Qt.Vertical

        TabView {
            anchors.margins: Qt.platform.os === "osx" ? 12 : 2
            tabPosition: Qt.TopEdge
            Layout.fillHeight: true
            Layout.minimumHeight: 300
            style: headerTabViewStyle

            Tab {
                id: importTab
                title: qsTr("Import")
                Import {
                    id: importItem
                    Component.onCompleted: {
                        importItem.tooltipWanted.connect(root.tooltipWanted)
                    }
                }
            }

            Tab {
                id: organizeTab
                title: qsTr("Organize")
                Organize {
                    id: organizeItem
                    Component.onCompleted: {
                        organizeItem.tooltipWanted.connect(root.tooltipWanted)
                    }
                }
            }

            Tab {
                id: editorTab
                property real defaultExposureComp: 0
                property real defaultWhitepoint: 2/1000
                property real defaultBlackpoint: 0
                property real defaultShadowsY: 0.25
                property real defaultHighlightsY: 0.75
                property real defaultFilmSize: 864
                property int defaultHighlightRecovery: 0
                property real defaultLayerMixConst: 0.2
                property bool defaultCaEnabled: false
                property real defaultTemperature: 5200
                property real defaultTint: 1
                property real defaultVibrance: 0
                property real defaultSaturation: 0
                property bool defaultOverdriveEnabled: false

                title: qsTr("Filmulate")
                Edit {
                    id: editItem
                    defaultExposureComp: editorTab.defaultExposureComp
                    defaultFilmSize: editorTab.defaultFilmSize
                    defaultWhitepoint: editorTab.defaultWhitepoint
                    defaultBlackpoint: editorTab.defaultBlackpoint
                    defaultShadowsY: editorTab.defaultShadowsY
                    defaultHighlightsY: editorTab.defaultHighlightsY
                    defaultHighlightRecovery: editorTab.defaultHighlightRecovery
                    defaultLayerMixConst: editorTab.defaultLayerMixConst
                    defaultCaEnabled: editorTab.defaultCaEnabled
                    defaultTemperature: editorTab.defaultTemperature
                    defaultTint: editorTab.defaultTint
                    defaultVibrance: editorTab.defaultVibrance
                    defaultSaturation: editorTab.defaultSaturation
                    defaultOverdriveEnabled: editorTab.defaultOverdriveEnabled

                    Component.onCompleted: {
                        editItem.tooltipWanted.connect(root.tooltipWanted)
                    }
                }
            }

            Tab {
                id: outputTab
                title: qsTr("Output")
            }
        }


        Rectangle {
            id: queue
            color: "#303030"
            height: 100
            Layout.minimumHeight: 50

            Queue {
                anchors.fill: parent
            }
        }
    }


    //Tooltip handling
    onTooltipWanted: {
        tooltipText.text = text
        tooltipCatcher.xInput = x
        tooltipCatcher.yInput = y
        tooltipCatcher.enabled = true
        tooltipCatcher.visible = true
    }

    MouseArea {
        id: tooltipCatcher
        acceptedButtons: Qt.NoButton
        anchors.fill: fillRect
        hoverEnabled: true
        enabled: false
        visible: false
        propagateComposedEvents: true
        property int xInput
        property int yInput
        property Item sourceItem

        onEnabledChanged: {
            if (enabled) {
                tooltipCatcher.setPosition(xInput, yInput)
            }
        }

        onPositionChanged: {
            tooltipCatcher.visible = false
            tooltipCatcher.enabled = false
        }
        onExited: {
            tooltipCatcher.visible = false
            tooltipCatcher.enabled = false
        }
        onWheel: {
            tooltipCatcher.visible = false
            tooltipCatcher.enabled = false
            wheel.accepted = false
        }

        Rectangle {
            id: tooltipBox
            color: "#EE303030"
            border.color: "#EE808080"
            border.width: 2
            radius: 10
            property int padding: 6
            property int maxWidth: 250
            property int minHeight: 30
            property int posPad: 10
            width: Math.min(maxWidth, tooltipText.contentWidth + 2*padding)
            height: tooltipText.contentHeight + 2*padding
            Text {
                id: tooltipText
                x: parent.padding
                y: parent.padding
                width: parent.maxWidth - 2*parent.padding
                wrapMode: Text.WordWrap
                color: "#FFFFFFFF"
            }
        }
        function setPosition(xIn, yIn) {
            if (tooltipBox.height + yIn < root.height) {
                tooltipBox.y = yIn
            }
            else {
                tooltipBox.y = yIn - tooltipBox.height
            }
            if (tooltipBox.width + xIn + tooltipBox.posPad < root.width) {
                tooltipBox.x = xIn + tooltipBox.posPad
            }
            else {
                tooltipBox.x = xIn - tooltipBox.width
            }
        }
    }

    //Global keyboard shortcut handling
    Action {
        id: fullscreenAction
        shortcut: "f11"
        onTriggered: {
            if (root.visibility !== 5) {
                root.tempVisibility = root.visibility
                root.visibility = 5//"FullScreen"
            }
            else {
                root.visibility = root.tempVisibility
            }
        }
    }

    //styles
    property Component headerTabViewStyle: TabViewStyle {
        tabOverlap: -4
        frameOverlap: -4

        frame: Rectangle {
            gradient: Gradient {
                GradientStop { color:"#505050"; position: 0 }
                GradientStop { color:"#000000"; position: 0 }
            }
        }

        tab: Rectangle {
            property int totalOverlap: tabOverlap * (control.count - 1)
            implicitWidth: Math.min ((styleData.availableWidth + totalOverlap) / control.count - 4, 100)
            implicitHeight: 30
            radius: 8
            border.color: styleData.selected ? "#B0B0B0" : "#808080"
            color: "#111111"
            Text {
                text: styleData.title
                color: "#ffffff"
                anchors.centerIn: parent
                font.bold: true
            }
        }
    }
}
