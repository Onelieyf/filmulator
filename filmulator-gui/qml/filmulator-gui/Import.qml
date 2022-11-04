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

    signal tooltipWanted(string text, int x, int y)

    ColumnLayout {
        id: importToolList
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
        ImportDirEntry {
            id: sourceDirEntry
            title: qsTr("Source Directory")
            tooltipText: qsTr("Select or type in the directory containing photos to be imported.")
            dirDialogTitle: qsTr("Select the directory containing the photos to import.")
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
        }

        ToolSlider {
            id: localOffset
            title: qsTr("Local UTC Offset")
            tooltipText: qsTr("Set this to the local UTC offset at the time and place of the photo's capture. This only affects what folders the photos are sorted into.")
            minimumValue: -14
            maximumValue: 14
            stepSize: 1
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
        ImportDirEntry {
            id: photoDirEntry
            title: qsTr("Destination Directory")
            tooltipText: qsTr("Select or type in the root directory of your photo file structure.")
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
        }
        ImportDirEntry {
            id: backupDirEntry
            title: qsTr("Backup Directory")
            tooltipText: qsTr("Select or type in the root directory of your backup file structure. If it doesn't exist, then no backup copies will be created.")
            dirDialogTitle: qsTr("Select the backup root directory")
            enteredText: settings.getPhotoBackupDir()
            onEnteredTextChanged: {
                importModel.backupDir = enteredText
                settings.photoBackupDir = enteredText
            }
            Component.onCompleted: {
                importModel.backupDir = enteredText
                photoDirEntry.tooltipWanted.connect(root.tooltipWanted)
            }
            uiScale: root.uiScale
        }

        ImportTextEntry {
            id: dirStructureEntry
            title: qsTr("Directory Structure")
            tooltipText: qsTr("Enter with y's, M's, and d's, slashes, and dashes the desired structure. example: \"/yyyy/MM/yyyy-MM-dd/\"")
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
        }

        ToolSwitch {
            id: enqueueSwitch
            text: qsTr("Enqueue imported photos")
            tooltipText: qsTr("As photos get imported, append them to the work queue.")
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
        ToolSlider {
            minimumValue: 0
            maximumValue: 10
            stepSize: 1
            defaultValue: 3
            uiScale: root.uiScale
            Component.onCompleted: {
                value = parent.test
            }
            onValueChanged: {
                parent.test = value
            }
        }
        property real test: 2

        ToolButton {
            id: importButton
            x: 0 * uiScale
            y: 0 * uiScale
            text: qsTr("Import")
            tooltipText: qsTr("Copy photos from the source directory into the destination (and optional backup), as well as adding them to the database.")
            width: parent.width
            height: 40 * uiScale
            action: Action{
                onTriggered: {
                    importModel.importDirectory_r(root.folderPath)
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
            Connections {
                target: importModel
                onProgressChanged: importProgress.value = importModel.progress
            }
            uiScale: root.uiScale
        }
    }
}
