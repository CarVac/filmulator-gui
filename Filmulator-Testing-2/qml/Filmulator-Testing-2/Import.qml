import QtQuick 2.2
import QtQuick.Controls 1.1
import QtQuick.Controls.Styles 1.1
import QtQuick.Layouts 1.1
import QtQuick.Dialogs 1.1
import "gui_components"

Rectangle {
    id: importRoot
    color: "#800080"
    anchors.fill: parent
    property string folderPath: ""

    onFolderPathChanged: {
        importDelay.start()
    }

    Timer {
        id: importDelay
        interval: 250
        onTriggered: {
            organizeModel.importDirectory(folderPath)
        }
    }

    ToolButton {
        id: openDirButton
        anchors.centerIn: parent
        text: qsTr("Import Directory")
        width: 200
        height: 40
        action: Action{
            onTriggered: {
                importDialog.open()
            }
        }
    }

    FileDialog {
        id: importDialog
        title: qsTr("Select a directory to import")
        selectFolder: true
        onAccepted: {
            importRoot.folderPath = fileUrl
            //organizeModel.importDirectory(folder)
        }
    }
}
