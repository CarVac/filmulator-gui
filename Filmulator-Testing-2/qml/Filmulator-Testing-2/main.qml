import QtQuick 2.2
import QtQuick.Controls 1.1
import QtQuick.Layouts 1.1
import QtQuick.Dialogs 1.1
import QtQuick.Controls.Styles 1.1
import "gui_components"

ApplicationWindow {
    id: mainwindow
    title: qsTr("Filmulator")
    width: 1024
    height: 768
    minimumHeight: 600
    minimumWidth:800

    Rectangle {
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
                title: qsTr("Import")
                Import {}
            }

            Tab {
                title: qsTr("Organize")
                Organize {}
            }

            Tab {
                id: editorTab
                property string location: ""
                property real exposureComp: 0
                property real defaultExposureComp: 0
                property real whitepoint: 2/1000
                property real defaultWhitepoint: 2/1000
                property real blackpoint: 0
                property real defaultBlackpoint: 0
                property real shadowsY: 0.25
                property real defaultShadowsY: 0
                property real highlightsY: 0.75
                property real defaultHighlightsY: 0.75
                property real filmSize: 864
                property real defaultFilmSize: 864
                property bool defaultCurve: true
                property int highlightRecovery: 0
                property int defaultHighlightRecovery: 0

                title: qsTr("Filmulate")
                Edit {
                    location: editorTab.location
                    exposureComp: editorTab.exposureComp
                    defaultExposureComp: editorTab.defaultExposureComp
                    filmSize: editorTab.filmSize
                    defaultFilmSize: editorTab.defaultFilmSize
                    whitepoint: editorTab.whitepoint
                    defaultWhitepoint: editorTab.defaultWhitepoint
                    blackpoint: editorTab.blackpoint
                    defaultBlackpoint: editorTab.defaultBlackpoint
                    shadowsY: editorTab.shadowsY
                    defaultShadowsY: editorTab.defaultShadowsY
                    highlightsY: editorTab.highlightsY
                    defaultHighlightsY: editorTab.defaultHighlightsY
                    defaultCurve: editorTab.defaultCurve
                    highlightRecovery: editorTab.highlightRecovery
                    defaultHighlightRecovery: editorTab.defaultHighlightRecovery

                    Connections {
                        target: openDialog
                        onAccepted: reset()
                    }
                }
            }

            Tab {
                title: qsTr("Output")
            }
        }

        Rectangle {
            id: queue
            color: "lightgreen"
            height: 100
            Layout.minimumHeight: 50

            ToolButton {
                id: openButton
                anchors.centerIn: parent
                text: qsTr("Open")
                width: 80
                height: 40
                action: Action {
                    onTriggered: {
                        openDialog.open()
                    }
                }
            }

            FileDialog {
                id: openDialog
                title: qsTr("Select a raw file")
                onAccepted: {
                    editorTab.location = fileUrl
                    /*
                    filmProvider.defaultToneCurveEnabled = editortab.defaultCurve
                    filmProvider.exposureComp = editortab.exposureComp
                    filmProvider.whitepoint = editortab.whitepoint
                    filmProvider.blackpoint = editortab.blackpoint
                    filmProvider.shadowsY = editortab.shadowsY
                    filmProvider.highlightsY = editortab.highlightsY
                    filmProvider.filmArea = editortab.filmSize
                    */
                }
            }

/*            Rectangle {
                id: textentryholder
                color: "#101010"
                height: 20
                width: 300
                radius: 5
                anchors.centerIn: parent
                TextInput {
                    id: filelocation
                    color: "#FFFFFF"
                    anchors.fill: parent
                    text: qsTr("Enter file path here")
                    onAccepted: {
                        editortab.location = filelocation.text
                        filmProvider.exposureComp = editortab.exposureComp;
                        filmProvider.whitepoint = editortab.whitepoint;
                    }
                }
            }*/
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
            implicitWidth: Math.min ((styleData.availableWidth + totalOverlap)/control.count - 4, 100)
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
