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
    property string folderPath: ""
    property string filePath: ""
    property bool sourceIsFolder: true
    property bool importInPlace: true

    signal tooltipWanted(string text, int x, int y)

    ColumnLayout {
        id: importDirToolList
        spacing: 0
        x: 3 * uiScale
        y: 3 * uiScale
        width: 300 * uiScale

        ToolSlider {
            id: cameraOffset
            title: qsTr("Camera UTC Offset")
            tooltipText: qsTr("Set this to the UTC offset of the camera in hours when it took the photos. Reminder: west is negative, east is positive.")
            minimumValue: -14
            maximumValue: 14
            stepSize: 1
            tickmarksEnabled: true
            tickmarkFactor: 6
            tickmarkOffset: 2
            minorTicksEnabled: true
            value: settings.getCameraTZ()
            defaultValue: settings.getCameraTZ()
            onValueChanged: {
                importModel.cameraTZ = value
                settings.cameraTZ = value
            }
            Component.onCompleted: {
                importModel.cameraTZ = value
                cameraOffset.tooltipWanted.connect(root.tooltipWanted)
            }
            uiScale: root.uiScale
        }

        RowLayout {
            id: sourceSelector
            spacing: 0
            width: parent.width
            height: 30 * uiScale

            ExclusiveGroup {id: sourceSelectorGroup}

            ToolRadioButton {
                id: sourceDirButton
                width: parent.width/2
                height: parent.height
                text: qsTr("Import Directory")
                tooltipText: qsTr("Import from a directory and all subdirectories.")
                checked: true
                exclusiveGroup: sourceSelectorGroup
                onCheckedChanged: {
                    if (checked) {
                        root.sourceIsFolder = true
                    }
                }
                Component.onCompleted: {
                    sourceDirButton.tooltipWanted.connect(root.tooltipWanted)
                }
                uiScale: root.uiScale
            }

            ToolRadioButton {
                id: sourceFileButton
                width: parent.width/2
                height: parent.height
                text: qsTr("Import Files")
                tooltipText: qsTr("Import one or more files.")
                exclusiveGroup: sourceSelectorGroup
                onCheckedChanged: {
                    if (checked) {
                        root.sourceIsFolder = false
                    }
                }
                Component.onCompleted: {
                    sourceFileButton.tooltipWanted.connect(root.tooltipWanted)
                }
                uiScale: root.uiScale
            }
        }

        ImportDirEntry {
            id: sourceDirEntry
            title: qsTr("Source Directory")
            tooltipText: qsTr("Select or type in the directory containing photos to be imported.")
            dirDialogTitle: qsTr("Select the directory containing the photos to import. It will only import raw files.")
            erroneous: importModel.emptyDir
            onEnteredTextChanged: {
                root.folderPath = enteredText
            }
            Connections {
                target: importModel
                onEmptyDirChanged: {
                    sourceDirEntry.erroneous = importModel.emptyDir
                }
            }
            Component.onCompleted: {
                sourceDirEntry.tooltipWanted.connect(root.tooltipWanted)
            }
            uiScale: root.uiScale
            visible: root.sourceIsFolder
        }

        ImportFileEntry {
            id: sourceFileEntry
            title: qsTr("Source Files")
            tooltipText: qsTr("Select one or more files to import.")
            fileDialogTitle: qsTr("Select the file(s) to import.")
            nameFilters: importModel.getNameFilters();
            onEnteredTextChanged: {
                root.filePath = enteredText
            }
            Component.onCompleted: {
                sourceFileEntry.tooltipWanted.connect(root.tooltipWanted)
            }
            uiScale: root.uiScale
            visible: !root.sourceIsFolder
        }

        ToolSlider {
            id: localOffset
            title: qsTr("Local UTC Offset")
            tooltipText: qsTr("Set this to the local UTC offset at the time and place of the photo's capture. This only affects what folders the photos are sorted into.")
            minimumValue: -14
            maximumValue: 14
            stepSize: 1
            tickmarksEnabled: true
            tickmarkFactor: 6
            tickmarkOffset: 2
            minorTicksEnabled: true
            value: settings.getImportTZ()
            defaultValue: settings.getImportTZ()
            onValueChanged: {
                importModel.importTZ = value
                settings.importTZ = value
            }
            Component.onCompleted: {
                importModel.importTZ = value
                localOffset.tooltipWanted.connect(root.tooltipWanted)
            }
            uiScale: root.uiScale
        }

        RowLayout {
            id: destSelector
            spacing: 0
            width: parent.width
            height: 30 * uiScale

            ExclusiveGroup {id: destSelectorGroup}

            ToolRadioButton {
                id: importAndMoveButton
                width: parent.width/2
                height: parent.height
                text: qsTr("Move to directory")
                tooltipText: qsTr("Copy files to a folder structure based on date and time of capture. This lets you create backup copies at the same time.")
                checked: true
                exclusiveGroup: destSelectorGroup
                onCheckedChanged: {
                    if (checked) {
                        root.importInPlace = false
                    }
                }
                Component.onCompleted: {
                    importAndMoveButton.tooltipWanted.connect(root.tooltipWanted)
                }
                uiScale: root.uiScale
            }

            ToolRadioButton {
                id: importInPlaceButton
                width: parent.width/2
                height: parent.height
                text: qsTr("Import in place")
                tooltipText: qsTr("Import files into the database without moving or copying them.")
                exclusiveGroup: destSelectorGroup
                onCheckedChanged: {
                    if (checked) {
                        root.importInPlace = true
                    }
                }
                Component.onCompleted: {
                    importInPlaceButton.tooltipWanted.connect(root.tooltipWanted)
                }

                uiScale: root.uiScale
            }
        }

        ImportDirEntry {
            id: photoDirEntry
            title: qsTr("Destination Directory")
            tooltipText: qsTr("Select or type in the root directory of your photo file structure. If it doesn\'t exist, then it will be created.")
            dirDialogTitle: qsTr("Select the destination root directory")
            enteredText: settings.getPhotoStorageDir()
            onEnteredTextChanged: {
                importModel.photoDir = enteredText
                settings.photoStorageDir = enteredText
            }
            Component.onCompleted: {
                importModel.photoDir = enteredText
                photoDirEntry.tooltipWanted.connect(root.tooltipWanted)
            }
            uiScale: root.uiScale
            visible: !root.importInPlace
        }
        ImportDirEntry {
            id: backupDirEntry
            title: qsTr("Backup Directory")
            tooltipText: qsTr("Select or type in the root directory of your backup file structure. If it doesn\'t exist, then no backup copies will be made.")
            dirDialogTitle: qsTr("Select the backup root directory")
            enteredText: settings.getPhotoBackupDir()
            onEnteredTextChanged: {
                importModel.backupDir = enteredText
                settings.photoBackupDir = enteredText
            }
            Component.onCompleted: {
                importModel.backupDir = enteredText
                backupDirEntry.tooltipWanted.connect(root.tooltipWanted)
            }
            uiScale: root.uiScale
            visible: !root.importInPlace
        }

        ImportTextEntry {
            id: dirStructureEntry
            title: qsTr("Directory Structure")
            tooltipText: qsTr("Enter using y's, M's, and d's, slashes, and other punctuation the desired structure. You can use single quotes to include words in the structure. For example:\n\"/yyyy/MM/yyyy-MM-dd/\"\n\"/yyyy/'Alaska'/yyyy-MM-dd/\"")
            enteredText: settings.getDirConfig()//"/yyyy/MM/yyyy-MM-dd/"
            onEnteredTextChanged: {
                importModel.dirConfig = enteredText
                settings.dirConfig = enteredText
            }
            Component.onCompleted: {
                importModel.dirConfig = enteredText
                dirStructureEntry.tooltipWanted.connect(root.tooltipWanted)
            }
            uiScale: root.uiScale
            visible: !root.importInPlace
        }

        ToolSwitch {
            id: appendSwitch
            text: qsTr("Append unique identifier")
            tooltipText: qsTr("This is recommended if you have multiple cameras of the same brand.\nIt appends an underscore and seven characters (derived from the file contents) to the filename, to prevent the same filename from being used twice in a folder.")
            isOn: settings.getAppendHash()
            onIsOnChanged: {
                importModel.appendHash = isOn
                settings.appendHash = isOn
            }
            defaultOn: false
            onResetToDefault: {
                importModel.appendHash = isOn
                settings.appendHash = isOn
            }
            Component.onCompleted: {
                importModel.appendHash = isOn
                appendSwitch.tooltipWanted.connect(root.tooltipWanted)
            }
            uiScale: root.uiScale
            visible: !root.importInPlace
        }

        ToolSwitch {
            id: enqueueSwitch
            text: qsTr("Enqueue imported photos")
            tooltipText: qsTr("As photos get imported, append them to the work queue.")
            isOn: settings.getEnqueue()
            onIsOnChanged: {
                importModel.enqueue = isOn
                settings.enqueue = isOn
            }
            defaultOn: false
            onResetToDefault: {
                importModel.enqueue = isOn
                settings.enqueue = isOn
            }
            Component.onCompleted: {
                importModel.enqueue = isOn
                enqueueSwitch.tooltipWanted.connect(root.tooltipWanted)
            }
            uiScale: root.uiScale
        }

        ToolButton {
            id: importButton
            x: 0 * uiScale
            y: 0 * uiScale
            text: qsTr("Import")
            tooltipText: qsTr("Start importing the selected file or folder. If importing is currently in progress, then the current file or folder will be imported after all current imports are complete.")
            width: parent.width
            height: 40 * uiScale
            onTriggered: {
                if (root.sourceIsFolder) {
                    importModel.importDirectory_r(root.folderPath)
                } else {
                    importModel.importFileList(root.filePath)
                }
            }
            Component.onCompleted: {
                importButton.tooltipWanted.connect(root.tooltipWanted)
            }
            uiScale: root.uiScale
        }

        FilmProgressBar {
            id: importProgress
            width: parent.width
            value: importModel.progress
            tooltipText: importModel.progressFrac
            Connections {
                target: importModel
                onProgressChanged: importProgress.value = importModel.progress
            }
            Component.onCompleted: {
                importProgress.tooltipWanted.connect(root.tooltipWanted)
            }

            uiScale: root.uiScale
        }
    }
}
