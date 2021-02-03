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
    property string folderPath: ""
    property string filePath: ""
    property bool sourceIsFolder: true
    property bool importInPlace: true
    property bool replaceLocation: false
    property bool replace: importInPlace && replaceLocation

    signal tooltipWanted(string text, int x, int y)

    ColumnLayout {
        id: importDirToolList
        spacing: 0
        x: 3 * uiScale
        y: 3 * uiScale
        width: 350 * uiScale

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

        Rectangle {
            id: sourceSelector
            width: parent.width
            height: 30 * root.uiScale
            color: Colors.darkGray

            property bool hovered: (sourceDirButton.hovered || sourceFileButton.hovered)

            ButtonGroup {
                id: sourceSelectorGroup
            }

            ToolRadioButton {
                id: sourceDirButton
                width: parent.width/2 - 2*uiScale
                height: parent.height
                x: 2*uiScale
                standalone: true
                text: qsTr("Import Directory")
                tooltipText: qsTr("Import from a directory and all subdirectories.")
                checked: true
                ButtonGroup.group: sourceSelectorGroup
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
                width: parent.width/2 - 2*uiScale
                height: parent.height
                x: parent.width/2
                standalone: true
                text: qsTr("Import Files")
                tooltipText: qsTr("Import one or more files.")
                ButtonGroup.group: sourceSelectorGroup
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
            warningTooltipText: empty ? qsTr("Choose a directory to import from.") : qsTr("You may be importing in place from a memory card. The photos will be lost if you format the card.\n\nDouble-click the error icon to proceed.")
            erroneous: (empty || (importInPlace && containsDCIM && !clearError))
            property bool containsDCIM: false
            property bool empty: enteredText === ""
            onEnteredTextChanged: {
                root.folderPath = enteredText
                containsDCIM = importModel.pathContainsDCIM(enteredText, false)
                clearError = false;
            }
            Component.onCompleted: {
                sourceDirEntry.tooltipWanted.connect(root.tooltipWanted)
            }
            uiScale: root.uiScale
            visible: root.sourceIsFolder
            highlight: sourceSelector.hovered
        }

        ImportFileEntry {
            id: sourceFileEntry
            title: qsTr("Source Files")
            tooltipText: qsTr("Select one or more files to import.")
            fileDialogTitle: qsTr("Select the file(s) to import.")
            warningTooltipText: !(importInPlace && containsDCIM && !clearError) ? qsTr("Choose a valid file.") : qsTr("You may be importing in place from a memory card. The photos will be lost if you format the card.\n\nDouble-click the error icon to proceed.")
            erroneous: (invalid || (importInPlace && containsDCIM && !clearError) || enteredText == "")
            property bool containsDCIM: false
            property bool invalid: false
            nameFilters: importModel.getNameFilters();
            onEnteredTextChanged: {
                root.filePath = enteredText
                containsDCIM = importModel.pathContainsDCIM(enteredText, true)
                clearError = false
                invalid = false //If it was invalid, we need to at least let them try to import again once they change the contents
            }
            Connections {
                target: importModel
                function onInvalidFileChanged() {
                    sourceFileEntry.invalid = importModel.invalidFile
                }
            }

            Component.onCompleted: {
                sourceFileEntry.tooltipWanted.connect(root.tooltipWanted)
            }
            uiScale: root.uiScale
            visible: !root.sourceIsFolder
            highlight: sourceSelector.hovered
        }

        Rectangle {
            id: destSelector
            width: parent.width
            height: 30 * uiScale
            color: Colors.darkGray

            property bool hovered: (importAndMoveButton.hovered || importInPlaceButton.hovered)

            ButtonGroup {
                id: destSelectorGroup
            }

            ToolRadioButton {
                id: importAndMoveButton
                width: parent.width/2 - 2*uiScale
                height: parent.height
                x: 2*uiScale
                standalone: true
                text: qsTr("Copy to directory")
                tooltipText: qsTr("When importing, copy files to a folder structure based on date and time of capture. This lets you create backup copies at the same time.")
                checked: true
                ButtonGroup.group: destSelectorGroup
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
                width: parent.width/2 - 2*uiScale
                height: parent.height
                x: parent.width/2
                standalone: true
                text: qsTr("Import in place")
                tooltipText: qsTr("Import files into the database without moving or copying them.")
                ButtonGroup.group: destSelectorGroup
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
            visible: !root.importInPlace
            highlight: destSelector.hovered
        }

        ImportDirEntry {
            id: photoDirEntry
            title: qsTr("Destination Directory")
            tooltipText: qsTr("Select or type in the root directory of your photo file structure. If it doesn\'t exist, then it will be created.")
            dirDialogTitle: qsTr("Select the destination root directory")
            warningTooltipText: empty ? qsTr("Choose a directory to move files to.") : qsTr("You do not have permissions to write in this directory. Please select another directory.")
            erroneous: (empty || notWritable)
            property bool notWritable: false
            property bool empty: enteredText === ""
            enteredText: settings.getPhotoStorageDir()
            onEnteredTextChanged: {
                importModel.photoDir = enteredText
                settings.photoStorageDir = enteredText
                notWritable = !importModel.pathWritable(enteredText)
            }
            Component.onCompleted: {
                importModel.photoDir = enteredText
                photoDirEntry.tooltipWanted.connect(root.tooltipWanted)
            }
            uiScale: root.uiScale
            visible: !root.importInPlace
            highlight: destSelector.hovered
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
            highlight: destSelector.hovered
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
            highlight: destSelector.hovered
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
            highlight: destSelector.hovered
        }

        ToolSwitch {
            id: replaceLocationSwitch
            text: qsTr("Update file locations")
            tooltipText: qsTr("If you need to remove files from your main working directory, enable this switch and run it on your backup to change the associated file locations.")
            isOn: false
            onIsOnChanged: {
                root.replaceLocation = replaceLocationSwitch.isOn
            }
            onResetToDefault: {
                replaceLocationSwitch.isOn = false
                root.replaceLocation = false
            }
            Component.onCompleted: {
                replaceLocationSwitch.tooltipWanted.connect(root.tooltipWanted)
            }
            uiScale: root.uiScale
            visible: root.importInPlace
            highlight: destSelector.hovered
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
            tooltipText: notDisabled ? qsTr("Start importing the selected file or folder. If importing is currently in progress, then the current file or folder will be imported after all current imports are complete.") : qsTr("Correct the errors that are highlighted above before importing.")
            width: parent.width
            height: 40 * uiScale
            // not disabled if dest is not erroneous, or if we're importing in place. It also needs to be not disabled if the appropriate source is not erroneous.
            notDisabled: ((!photoDirEntry.erroneous || root.importInPlace) && (root.sourceIsFolder ? !sourceDirEntry.erroneous : !sourceFileEntry.erroneous))
            onTriggered: {
                if (root.sourceIsFolder) {
                    importModel.importDirectory_r(root.folderPath, root.importInPlace, root.replace)
                    sourceDirEntry.clearError = false
                } else {
                    importModel.importFileList(root.filePath, root.importInPlace, root.replace)
                    sourceFileEntry.clearError = false
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
                function onProgressChanged() { importProgress.value = importModel.progress }
            }
            Component.onCompleted: {
                importProgress.tooltipWanted.connect(root.tooltipWanted)
            }

            uiScale: root.uiScale
        }
    }

    ColumnLayout {
        id: scenarioList
        spacing: 0
        x: 600 * uiScale
        y: 20 * uiScale
        width: 350 * uiScale

        Rectangle {
            id: scenarioLabelBox
            width: parent.width
            height: 20 * uiScale
            color: Colors.darkGray

            Text {
                id: scenarioLabelText
                x: 3 * uiScale
                y: 0
                width: parent.width-x
                height: parent.height
                color: "white"
                horizontalAlignment: Text.AlignLeft
                verticalAlignment: Text.AlignVCenter
                font.pixelSize: 12.0 * uiScale
                text: qsTr("Common import scenarios:")
            }
        }

        ToolButton {
            id: memoryCardScenario
            width: parent.width
            height: 60 * uiScale
            text: qsTr("Import new photos from a memory card","Put a line break in if it gets much longer")
            tooltipText: qsTr("This setup will copy photos from a memory card to a destination directory, and load all newly imported photos into the queue.")
            highlight: root.sourceIsFolder && !root.importInPlace && appendSwitch.isOn && enqueueSwitch.isOn
            noOutlineClick: true
            onTriggered: {
                sourceDirButton.checked = true
                importAndMoveButton.checked = true
                appendSwitch.isOn = true
                enqueueSwitch.isOn = true
                console.log("sourceisfolder " + root.sourceIsFolder)
                console.log("import in place " + root.importInPlace)
                console.log("append " + importModel.appendHash)
                console.log("enqueue " + importModel.enqueue)
            }
            Component.onCompleted: {
                memoryCardScenario.tooltipWanted.connect(root.tooltipWanted)
            }
            uiScale: root.uiScale
        }
        ToolButton {
            id: existingPhotosScenario
            width: parent.width
            height: 60 * uiScale
            text: qsTr("Import existing photos")
            tooltipText: qsTr("This setup will import photos that are already on your computer, and load all newly imported photos into the queue.")
            highlight: root.importInPlace && !replaceLocationSwitch.isOn && enqueueSwitch.isOn
            noOutlineClick: true
            onTriggered: {
                importInPlaceButton.checked = true
                replaceLocationSwitch.isOn = false
                enqueueSwitch.isOn = true
                console.log("import in place " + root.importInPlace)
                console.log("replace location " + root.replace)
                console.log("enqueue " + importModel.enqueue)
            }
            Component.onCompleted: {
                existingPhotosScenario.tooltipWanted.connect(root.tooltipWanted)
            }
            uiScale: root.uiScale
        }
        ToolButton {
            id: findBackupScenario
            width: parent.width
            height: 60 * uiScale
            text: qsTr("Update locations of files that have moved","Put a line break in if it gets much longer")
            tooltipText: qsTr("If a photo in the database has its raw file moved, use this setup to re-import the photo. It will not load anything into the queue.\n\nThis is useful when you remove photos from your main directory and want to work from a backup location.")
            highlight: root.sourceIsFolder && root.importInPlace && replaceLocationSwitch.isOn && !enqueueSwitch.isOn
            noOutlineClick: true
            onTriggered: {
                sourceDirButton.checked = true
                importInPlaceButton.checked = true
                replaceLocationSwitch.isOn = true
                enqueueSwitch.isOn = false
                console.log("sourceisfolder " + root.sourceIsFolder)
                console.log("import in place " + root.importInPlace)
                console.log("replace location " + root.replace)
                console.log("enqueue " + importModel.enqueue)
            }
            Component.onCompleted: {
                findBackupScenario.tooltipWanted.connect(root.tooltipWanted)
            }
            uiScale: root.uiScale
        }
        ToolButton {
            id: enqueueFileScenario
            width: parent.width
            height: 60 * uiScale
            text: qsTr("Bring previously-imported files into the queue","Put a line break in if it gets much longer")
            tooltipText: qsTr("If a file is in the database but you don't know when it was taken, just re-import it with this setup to load it into the queue.")
            highlight: root.importInPlace && replaceLocationSwitch.isOn && enqueueSwitch.isOn
            noOutlineClick: true
            onTriggered: {
                importInPlaceButton.checked = true
                replaceLocationSwitch.isOn = true
                enqueueSwitch.isOn = true
                console.log("import in place " + root.importInPlace)
                console.log("replace location " + root.replace)
                console.log("enqueue " + importModel.enqueue)
            }
            Component.onCompleted: {
                findBackupScenario.tooltipWanted.connect(root.tooltipWanted)
            }
            uiScale: root.uiScale
        }
    }
}
