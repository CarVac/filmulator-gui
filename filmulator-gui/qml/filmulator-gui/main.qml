import QtQuick 2.2
import QtQuick.Controls 1.2
import QtQuick.Layouts 1.1
import QtQuick.Dialogs 1.1
import QtQuick.Controls.Styles 1.2
import "gui_components"
import "colors.js" as Colors

ApplicationWindow {
    id: root
    title: qsTr("Filmulator")
    property real uiScale: settings.getUiScale()
    property int tempVisibility
    width: 1366 * uiScale
    height: 768 * uiScale
    minimumHeight: 400 * uiScale
    minimumWidth:700 * uiScale

    signal tooltipWanted(string text, int x, int y)
    signal imageURL(string newURL)

    Rectangle {
        id: fillRect
        anchors.fill: parent
        color: Colors.darkGray
    }

    SplitView {
        anchors.fill: parent
        orientation: Qt.Vertical

        TabView {
            id: tabs
            anchors.margins: (Qt.platform.os === "osx" ? 12 : 2) * uiScale
            tabPosition: Qt.TopEdge
            Layout.fillHeight: true
            Layout.minimumHeight: 200 * uiScale
            style: headerTabViewStyle

            Tab {
                id: importTab
                title: qsTr("Import")
                active: true
                Import {
                    id: importItem
                    Component.onCompleted: {
                        importItem.tooltipWanted.connect(root.tooltipWanted)
                    }
                    uiScale: root.uiScale
                }
            }

            Tab {
                id: organizeTab
                title: qsTr("Organize")
                active: true
                Organize {
                    id: organizeItem
                    Component.onCompleted: {
                        organizeItem.tooltipWanted.connect(root.tooltipWanted)
                    }
                    uiScale: root.uiScale
                }
            }

            Tab {
                id: editorTab
                title: qsTr("Filmulate")
                active: true
                Edit {
                    id: editItem
                    Component.onCompleted: {
                        editItem.tooltipWanted.connect(root.tooltipWanted)
                        editItem.imageURL.connect(root.imageURL)
                    }
                    uiScale: root.uiScale
                }
            }

            Tab {
                id: outputTab
                active: true
                title: qsTr("Output")
            }

            Tab {
                id: settingsTab
                title: qsTr("Settings")
                active: true
                Settings {
                    id: settingsItem
                    Component.onCompleted: {
                        settingsItem.tooltipWanted.connect(root.tooltipWanted)
                    }
                    uiScale: root.uiScale
                }
            }
        }


        Rectangle {
            id: queue
            color: Colors.darkGray
            height: 100 * uiScale
            Layout.minimumHeight: 50 * uiScale

            Queue {
                id: queueItem
                anchors.fill: parent
                uiScale: root.uiScale
                Connections {
                    target: root
                    onImageURL: {
                        //This is for telling the queue to grab the same image as the main editor
                        // so that the queue matches what you see in the editor.
                        console.log("main.qml queueItem update url")
                        queueItem.url = newURL;
                    }
                }
                Component.onCompleted: {
                    queueItem.tooltipWanted.connect(root.tooltipWanted)
                }
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
            color: Colors.transDarkGray
            border.color: Colors.transMiddleGray
            border.width: 2 * uiScale
            radius: 10 * uiScale
            property int padding: 6 * uiScale
            property int maxWidth: 250 * uiScale
            property int minHeight: 30 * uiScale
            property int posPad: 10 * uiScale
            width: Math.min(maxWidth, tooltipText.contentWidth + 2*padding)
            height: tooltipText.contentHeight + 2*padding
            z: 10
            Text {
                id: tooltipText
                x: parent.padding
                y: parent.padding
                width: parent.maxWidth - 2*parent.padding
                wrapMode: Text.WordWrap
                color: "#FFFFFFFF"
                font.pixelSize: 12.0 * uiScale
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
        tabOverlap: 0 * uiScale
        frameOverlap: 0 * uiScale

        frame: Rectangle { //The contents of the tab.
            color: Colors.lowGrayL
        }

        tab: Item {
            property int totalOverlap: tabOverlap * (control.count - 1)
            implicitWidth: Math.min (((styleData.availableWidth + totalOverlap) / control.count) - control.count, 105 * uiScale)
            implicitHeight: 37 * uiScale
            Rectangle {
                x: 4 * uiScale
                y: 4 * uiScale
                width: parent.implicitWidth - 5 * uiScale
                height: parent.implicitHeight - 8 * uiScale
                radius: 8 * uiScale
                border.width: 1 * uiScale
                border.color: styleData.selected ? Colors.whiteGrayH : Colors.middleGray
                color: Colors.blackGray
                gradient: Gradient {
                    GradientStop {color: styleData.selected ? "#000000" : "#222222"; position: 0.0}
                    GradientStop {color: "#111111";                                  position: 0.3}
                    GradientStop {color: "#111111";                                  position: 0.7}
                    GradientStop {color: styleData.selected ? "#222222" : "#000000"; position: 1.0}
                }

                Text {
                    text: styleData.title
                    color: "white"
                    anchors.centerIn: parent
                    font.bold: true
                    font.pixelSize: 12.0 * uiScale
                }
            }
        }
    }
}
