import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import QtQuick.Dialogs 1.3
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
    property bool cropping: false


    onClosing: {
        close.accepted = false
        importModel.exitWorker()
        filmProvider.exitWorker()
        close.accepted = true
    }

    Rectangle {
        id: fillRect
        anchors.fill: parent
        color: Colors.darkGray
    }

    SlimSplitView {
        anchors.fill: parent
        orientation: Qt.Vertical

        Item {
            id: tabContainer
            Layout.fillHeight: true
            Layout.minimumHeight: 200 * uiScale
            TabBar {
                id: tabs

                background:  Rectangle {
                    //needs to be invisible
                    color: "#00000000"
                }
                property real tabwidth: 105
                property real tabheight: 61
                property real rectx: 4
                property real recty: 28.5
                property real rectheight: 54

                TabButton {
                    id: importButton
                    text: qsTr("Import")
                    width: tabs.tabwidth * uiScale
                    height: tabs.tabheight * uiScale
                    background: Rectangle {
                        x: tabs.rectx * uiScale
                        //WTF WHY!? why doesn't it scale properly if I don't do this instead of just uiscale?
                        y: tabs.recty * (1+1.2*(uiScale-1))
                        width: parent.width - 5*uiScale
                        height: tabs.rectheight * uiScale
                        radius: 8 * uiScale
                        border.width: 1 * uiScale
                        border.color: parent.checked ? Colors.lightOrange : Colors.middleGray
                        gradient: Gradient {
                            GradientStop { color: importButton.checked ? "#222222" : "#000000"; position: 0.0 }
                            GradientStop { color: "#111111";                                    position: 0.15}
                            GradientStop { color: "#111111";                                    position: 1.0 }
                        }
                        Item {
                            x: 0
                            y: 1.5 * uiScale
                            width: parent.width
                            height: (parent.height/2)*1.1
                            Text {
                                color: importButton.checked ? "white" : Colors.brightGray
                                text: importButton.text
                                anchors.centerIn: parent
                                //anchors.horizontalCenter: parent.horizontalCenter
                                //y: 7.75 * uiScale
                                horizontalAlignment: Text.AlignHCenter
                                verticalAlignment: Text.AlignVCenter
                                font.bold: true
                                font.pixelSize: 12.0 * uiScale
                            }
                            FilmProgressBar {
                                id: importTabProgress
                                mini: true
                                anchors.horizontalCenter: parent.horizontalCenter
                                y: 22 * uiScale
                                width: parent.width - 8*uiScale
                                height: 6 * uiScale
                                value: importModel.progress
                                visible: (value > 0 && value < 1)
                                Connections {
                                    target: importModel
                                    onProgressChanged: importTabProgress.value = importModel.progress
                                }
                                uiScale: root.uiScale
                            }
                        }
                    }
                    contentItem: Item {
                        //dummy to hide the default text
                    }
                }
                TabButton {
                    id: organizeButton
                    text: qsTr("Organize")
                    width: tabs.tabwidth * uiScale
                    height: tabs.tabheight * uiScale
                    background: Rectangle {
                        x: tabs.rectx * uiScale
                        //WTF WHY!? why doesn't it scale properly if I don't do this instead of just uiscale?
                        y: tabs.recty * (1+1.2*(uiScale-1))
                        width: parent.width - 5*uiScale
                        height: tabs.rectheight * uiScale
                        radius: 8 * uiScale
                        border.width: 1 * uiScale
                        border.color: parent.checked ? Colors.lightOrange : Colors.middleGray
                        gradient: Gradient {
                            GradientStop { color: organizeButton.checked ? "#222222" : "#000000"; position: 0.0 }
                            GradientStop { color: "#111111";                                      position: 0.15}
                            GradientStop { color: "#111111";                                      position: 1.0 }
                        }
                        Item {
                            x: 0
                            y: 1.5 * uiScale
                            width: parent.width
                            height: (parent.height/2)*1.1
                            Text {
                                color: organizeButton.checked ? "white" : Colors.brightGray
                                text: organizeButton.text
                                anchors.centerIn: parent
                                horizontalAlignment: Text.AlignHCenter
                                verticalAlignment: Text.AlignVCenter
                                font.bold: true
                                font.pixelSize: 12.0 * uiScale
                            }
                        }
                    }
                    contentItem: Item {
                        //dummy to hide the default text
                    }
                }
                TabButton {
                    id: filmulateButton
                    text: qsTr("Filmulate")
                    width: tabs.tabwidth * uiScale
                    height: tabs.tabheight * uiScale
                    background: Rectangle {
                        x: tabs.rectx * uiScale
                        //WTF WHY!? why doesn't it scale properly if I don't do this instead of just uiscale?
                        y: tabs.recty * (1+1.2*(uiScale-1))
                        width: parent.width - 5*uiScale
                        height: tabs.rectheight * uiScale
                        radius: 8 * uiScale
                        border.width: 1 * uiScale
                        border.color: parent.checked ? Colors.lightOrange : Colors.middleGray
                        gradient: Gradient {
                            GradientStop { color: filmulateButton.checked ? "#222222" : "#000000"; position: 0.0 }
                            GradientStop { color: "#111111";                                       position: 0.15}
                            GradientStop { color: "#111111";                                       position: 1.0 }
                        }
                        Item {
                            x: 0
                            y: 1.5 * uiScale
                            width: parent.width
                            height: (parent.height/2)*1.1
                            Text {
                                color: filmulateButton.checked ? "white" : Colors.brightGray
                                text: filmulateButton.text
                                anchors.centerIn: parent
                                horizontalAlignment: Text.AlignHCenter
                                verticalAlignment: Text.AlignVCenter
                                font.bold: true
                                font.pixelSize: 12.0 * uiScale
                            }
                        }
                    }
                    contentItem: Item {
                        //dummy to hide the default text
                    }
                }
                TabButton {
                    id: settingsButton
                    text: qsTr("Settings")
                    width: tabs.tabwidth * uiScale
                    height: tabs.tabheight * uiScale
                    background: Rectangle {
                        x: tabs.rectx * uiScale
                        //WTF WHY!? why doesn't it scale properly if I don't do this instead of just uiscale?
                        y: tabs.recty * (1+1.2*(uiScale-1))
                        width: parent.width - 5*uiScale
                        height: tabs.rectheight * uiScale
                        radius: 8 * uiScale
                        border.width: 1 * uiScale
                        border.color: parent.checked ? Colors.lightOrange : Colors.middleGray
                        gradient: Gradient {
                            GradientStop { color: settingsButton.checked ? "#222222" : "#000000"; position: 0.0 }
                            GradientStop { color: "#111111";                                      position: 0.15}
                            GradientStop { color: "#111111";                                      position: 1.0 }
                        }
                        Item {
                            x: 0
                            y: 1.5 * uiScale
                            width: parent.width
                            height: (parent.height/2)*1.1
                            Text {
                                color: settingsButton.checked ? "white" : Colors.brightGray
                                text: settingsButton.text
                                anchors.centerIn: parent
                                horizontalAlignment: Text.AlignHCenter
                                verticalAlignment: Text.AlignVCenter
                                font.bold: true
                                font.pixelSize: 12.0 * uiScale
                            }
                        }
                    }
                    contentItem: Item {
                        //dummy to hide the default text
                    }
                }
            }

            StackLayout {
                id: mainContent
                x: 0
                y: Math.ceil(36 * uiScale)
                width: parent.width
                height: parent.height-y

                currentIndex: tabs.currentIndex

                Import {
                    id: importItem
                    Component.onCompleted: {
                        importItem.tooltipWanted.connect(root.tooltipWanted)
                    }
                    uiScale: root.uiScale
                }

                Organize {
                    id: organizeItem
                    Component.onCompleted: {
                        organizeItem.tooltipWanted.connect(root.tooltipWanted)
                    }
                    uiScale: root.uiScale
                }

                Edit {
                    id: editItem
                    Component.onCompleted: {
                        editItem.tooltipWanted.connect(root.tooltipWanted)
                        editItem.imageURL.connect(root.imageURL)
                    }
                    onRequestingCroppingChanged: {
                        root.cropping = editItem.requestingCropping
                    }
                    uiScale: root.uiScale
                }

                Settings {
                    id: settingsItem
                    Component.onCompleted: {
                        settingsItem.tooltipWanted.connect(root.tooltipWanted)
                    }
                    uiScale: root.uiScale
                }
            }
            Rectangle {
                id: tabBottomBorder
                x: 0
                y: 36 * uiScale
                width: parent.width
                height: 1 * uiScale
                color: Colors.whiteGrayH
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
                        if (!root.cropping) {
                            console.log("main.qml queueItem update url")
                            queueItem.url = newURL;
                        } else {
                            console.log("REMOVEME queueItem we're cropping now")
                        }
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
            color: Colors.darkGray
            border.color: Colors.lowGray
            border.width: 2 * uiScale
            radius: 10 * uiScale
            property int padding: 6 * uiScale
            property int maxWidth: 250 * uiScale
            property int minHeight: 30 * uiScale
            property int posPad: 10 * uiScale
            width: Math.min(maxWidth, tooltipText.contentWidth + 2*padding)
            //For some reason, this switch needs to exist in height so that 1-line tooltips are displayed correctly the first time.
            height: parent.visible ? (tooltipText.contentHeight + 2*padding) : 12
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
    /*
    property Component headerTabViewStyle: TabViewStyle {
        tabOverlap: 0 * uiScale
        frameOverlap: 0 * uiScale

        frame: Rectangle { //The contents of the tab.
            color: Colors.lowGrayL
        }

        tab: Item {
            property int totalOverlap: tabOverlap * (control.count - 1)
            implicitWidth: Math.min (((styleData.availableWidth + totalOverlap) / control.count) - control.count, 105 * uiScale)
            implicitHeight: Math.round(37 * uiScale)
            Rectangle {
                x: 4 * uiScale
                y: 4 * uiScale
                width: parent.implicitWidth - 5 * uiScale
                height: (parent.implicitHeight - Math.round(8 * uiScale))*2
                radius: 8 * uiScale
                border.width: 1 * uiScale
                border.color: styleData.selected ? Colors.lightOrange : Colors.middleGray
                color: Colors.blackGray
                gradient: Gradient {
                    GradientStop {color: !styleData.selected ? "#000000" : "#222222"; position: 0.0}
                    GradientStop {color: "#111111";                                  position: 0.15}
                    GradientStop {color: "#111111";                                  position: 1.0}
                }

                Item {
                    x: 0
                    y: 0.5 * uiScale
                    width: parent.width
                    height: (parent.height/2)*1.1
                    Text {
                        text: styleData.title
                        color: styleData.selected ? "white" : Colors.brightGrayH
                        anchors.centerIn: parent
                        font.bold: true
                        font.pixelSize: 12.0 * uiScale
                    }
                }
            }
        }
    }*/
}
