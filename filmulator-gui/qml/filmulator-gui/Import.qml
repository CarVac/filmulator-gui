import QtQuick 2.2
import QtQuick.Controls 1.1
import QtQuick.Controls.Styles 1.2
import QtQuick.Layouts 1.1
import QtQuick.Dialogs 1.1
import "gui_components"

Rectangle {
    id: root
    color: "#202020"
    anchors.fill: parent
    property string folderPath: ""

    signal tooltipWanted( string text, int x, int y )

    ColumnLayout {
        spacing: 0
        x: 3
        y: 3
        width: 300

        ToolSlider {
            id: cameraOffset
            title: qsTr( "Camera UTC Offset" )
            tooltipText: qsTr( "Set this to the UTC offset of the camera in hours when it took the photos. Reminder: west is negative, east is positive." )
            minimumValue: -14
            maximumValue: 14
            stepSize: 1
            defaultValue: settings.getCameraTZ()
            onValueChanged: {
                organizeModel.cameraTZ = value
                settings.cameraTZ = value
            }
            Component.onCompleted: {
                cameraOffset.tooltipWanted.connect( root.tooltipWanted )
            }
        }
        ImportDirEntry {
            id: sourceDirEntry
            title: qsTr( "Source Directory" )
            tooltipText: qsTr( "Select or type in the directory containing photos to be imported." )
            dirDialogTitle: qsTr( "Select the directory containing the photos to import." )
            onEnteredTextChanged: {
                root.folderPath = enteredText
            }
            Component.onCompleted: {
                sourceDirEntry.tooltipWanted.connect( root.tooltipWanted )
            }
        }

        ToolSlider {
            id: localOffset
            title: qsTr( "Local UTC Offset" )
            tooltipText: qsTr( "Set this to the local UTC offset at the time and place of the photo's capture. This only affects what folders the photos are sorted into." )
            minimumValue: -14
            maximumValue: 14
            stepSize: 1
            defaultValue: settings.getImportTZ()
            onValueChanged: {
                organizeModel.importTZ = value
                settings.importTZ = value
            }
            Component.onCompleted: {
                localOffset.tooltipWanted.connect( root.tooltipWanted )
            }
        }
        ImportDirEntry {
            id: photoDirEntry
            title: qsTr( "Destination Directory" )
            tooltipText: qsTr( "Select or type in the root directory of your photo file structure." )
            dirDialogTitle: qsTr( "Select the destination root directory" )
            enteredText: settings.getPhotoStorageDir()
            onEnteredTextChanged: {
                organizeModel.photoDir = enteredText
                settings.photoStorageDir = enteredText
            }
            Component.onCompleted: {
                photoDirEntry.tooltipWanted.connect( root.tooltipWanted )
            }
        }
        ImportDirEntry {
            id: backupDirEntry
            title: qsTr( "Backup Directory" )
            tooltipText: qsTr( "Select or type in the root directory of your backup file structure. If it doesn't exist, then no backup copies will be created.")
            dirDialogTitle: qsTr( "Select the backup root directory" )
            enteredText: settings.getPhotoBackupDir()
            onEnteredTextChanged: {
                organizeModel.backupDir = enteredText
                settings.photoBackupDir = enteredText
            }
            Component.onCompleted: {
                photoDirEntry.tooltipWanted.connect( root.tooltipWanted )
            }
        }

        ImportTextEntry {
            id: dirStructureEntry
            title: qsTr( "Directory Structure" )
            tooltipText: qsTr( "Enter with y's, M's, and d's, slashes, and dashes the desired structure. example: \"/yyyy/MM/yyyy-MM-dd/\"")
            enteredText: settings.getDirConfig()//"/yyyy/MM/yyyy-MM-dd/"
            onEnteredTextChanged: {
                //For some reason, when there's a default for enteredText, it tries to change this
                // before the resource organizeModel is defined. So in case no changes are made,
                // this has to be done again when they hit the import button.
                organizeModel.dirConfig = enteredText
                settings.dirConfig = enteredText
            }
            Component.onCompleted: {
                dirStructureEntry.tooltipWanted.connect( root.tooltipWanted )
            }
        }
    }

    ToolButton {
        id: importButton
        anchors.centerIn: parent
        text: qsTr( "Import" )
        width: 200
        height: 40
        action: Action{
            onTriggered: {
                organizeModel.dirConfig = dirStructureEntry.enteredText
                organizeModel.importDirectory_r( root.folderPath )
            }
        }
    }
}
