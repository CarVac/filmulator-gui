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
                id: editortab
                property string location: ""
                property real rolling: 0
                property real exposureComp: 0
                property real whitepoint: 2/1000
                property real blackpoint: 0
                property real shadowsY: 0.25
                property real highlightsY: 0.75
                property real filmSize: 864
                property bool defaultCurve: true

                title: qsTr("Filmulate")
                Edit {
                    location: editortab.location
                    index: editortab.rolling
                    exposureComp: editortab.exposureComp
                    filmSize: editortab.filmSize
                    whitepoint: editortab.whitepoint
                    blackpoint: editortab.blackpoint
                    shadowY: editortab.shadowsY
                    highlightY: editortab.highlightsY
                    defaultCurve: editortab.defaultCurve
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
                    editortab.location = fileUrl
                    filmProvider.exposureComp = editortab.exposureComp
                    filmProvider.whitepoint = editortab.whitepoint
                    filmProvider.blackpoint = editortab.blackpoint
                    filmProvider.shadowsY = editortab.shadowsY
                    filmProvider.highlightsY = editortab.highlightsY
                    filmProvider.filmArea = editortab.filmSize

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
