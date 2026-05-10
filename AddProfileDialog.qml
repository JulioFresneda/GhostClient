import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Dialog {
    id: addProfileDialog
    modal: true
    visible: false
    width: 680
    height: 580
    x: (parent.width - width) / 2
    y: (parent.height - height) / 2
    dim: true
    padding: 0

    property var profileName: ""
    property int pictureID: 1

    signal addProfileClicked()

    // Dimmed backdrop
    Overlay.modal: Rectangle {
        color: "#CC000000"
    }

    background: Rectangle {
        color: "#F2050505"
        radius: 0
        border.color: "#33FFFFFF"
        border.width: 1
    }

    contentItem: ColumnLayout {
        spacing: 18
        anchors.fill: parent
        anchors.margins: 28

        // ─── Name field with thin underline ───
        Item {
            Layout.fillWidth: true
            Layout.preferredHeight: 56

            TextField {
                id: profileNameField
                anchors.fill: parent
                placeholderText: qsTr("Name your ghost")
                color: "white"
                placeholderTextColor: "#66FFFFFF"
                font.pixelSize: 22
                focus: true
                background: Item {}
                topPadding: 8
                bottomPadding: 12
                Keys.onReturnPressed: imageGrid.forceActiveFocus()
            }

            Rectangle {
                id: underline
                anchors.bottom: parent.bottom
                anchors.left: parent.left
                anchors.right: parent.right
                height: 2
                color: profileNameField.activeFocus ? colors.superGreen : "#33FFFFFF"
                Behavior on color { ColorAnimation { duration: 180 } }
            }
        }

        // ─── Picture grid ───
        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            color: "#0DFFFFFF"
            radius: 0
            border.color: "#1AFFFFFF"
            border.width: 1

            GridView {
                id: imageGrid
                anchors.fill: parent
                anchors.margins: 12
                cellWidth: 100
                cellHeight: 100
                model: 36
                clip: true
                focus: false

                delegate: Item {
                    width: imageGrid.cellWidth
                    height: imageGrid.cellHeight
                    focus: GridView.isCurrentItem

                    Rectangle {
                        id: cell
                        anchors.centerIn: parent
                        width: 84
                        height: 84
                        radius: 0
                        color: "transparent"
                        border.color: (index + 1) === addProfileDialog.pictureID
                                      ? colors.superGreen
                                      : (cellMouse.containsMouse ? "#88FFFFFF" : "transparent")
                        border.width: (index + 1) === addProfileDialog.pictureID ? 3 : 2
                        scale: (index + 1) === addProfileDialog.pictureID ? 1.05 : 1.0

                        Behavior on scale { NumberAnimation { duration: 150; easing.type: Easing.OutQuad } }
                        Behavior on border.color { ColorAnimation { duration: 150 } }

                        Image {
                            anchors.fill: parent
                            anchors.margins: 4
                            sourceSize.width: 120
                            sourceSize.height: 120
                            smooth: true
                            source: "qrc:/media/ghosts/" + (index + 1) + ".png"
                        }

                        MouseArea {
                            id: cellMouse
                            anchors.fill: parent
                            hoverEnabled: true
                            cursorShape: Qt.PointingHandCursor
                            onClicked: addProfileDialog.pictureID = index + 1
                        }
                    }

                    Keys.onReturnPressed: rowbuttons.forceActiveFocus()
                    Keys.onRightPressed: moveToNextItem(1)
                    Keys.onLeftPressed: moveToPreviousItem(1)
                    Keys.onUpPressed: moveToPreviousItem(6)
                    Keys.onDownPressed: moveToNextItem(6)

                    function moveToNextItem(distance) {
                        if (addProfileDialog.pictureID < imageGrid.count - distance + 1) {
                            addProfileDialog.pictureID += distance
                        }
                    }
                    function moveToPreviousItem(distance) {
                        if (addProfileDialog.pictureID > distance) {
                            addProfileDialog.pictureID -= distance
                        }
                    }
                }
            }
        }

        // ─── Buttons ───
        RowLayout {
            id: rowbuttons
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignRight
            spacing: 12

            property bool addselected: true
            Keys.onReturnPressed: {
                if (addselected) {
                    addProfileDialog.profileName = profileNameField.text
                    addProfileDialog.addProfileClicked()
                    addProfileDialog.close()
                } else {
                    addProfileDialog.close()
                    addProfileDialog.profileName = ""
                    addProfileDialog.pictureID = 1
                }
            }
            Keys.onLeftPressed: addselected = false
            Keys.onRightPressed: addselected = true

            Item { Layout.fillWidth: true }

            // Cancel — outlined pill
            Button {
                id: cancelBtn
                text: qsTr("Cancel")
                font.pixelSize: 15
                Layout.preferredWidth: 130
                Layout.preferredHeight: 42
                onClicked: {
                    addProfileDialog.close()
                    addProfileDialog.profileName = ""
                    addProfileDialog.pictureID = 1
                }
                background: Rectangle {
                    color: cancelBtn.hovered ? "#1AFFFFFF" : "transparent"
                    radius: 0
                    border.color: rowbuttons.addselected ? "#33FFFFFF" : colors.superGreen
                    border.width: 1
                    Behavior on color { ColorAnimation { duration: 150 } }
                    Behavior on border.color { ColorAnimation { duration: 150 } }
                }
                contentItem: Text {
                    text: cancelBtn.text
                    color: "white"
                    font: cancelBtn.font
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }
            }

            // Add — filled green pill
            Button {
                id: addBtn
                text: qsTr("Add ghost")
                font.pixelSize: 15
                font.bold: true
                Layout.preferredWidth: 150
                Layout.preferredHeight: 42
                enabled: profileNameField.text.length > 0
                onClicked: {
                    addProfileDialog.profileName = profileNameField.text
                    addProfileDialog.addProfileClicked()
                    addProfileDialog.close()
                }
                background: Rectangle {
                    color: !addBtn.enabled
                           ? "#33419A38"
                           : (addBtn.hovered ? colors.superGreen : colors.green)
                    radius: 0
                    border.color: rowbuttons.addselected ? colors.superGreen : "transparent"
                    border.width: 2
                    Behavior on color { ColorAnimation { duration: 150 } }
                }
                contentItem: Text {
                    text: addBtn.text
                    color: "white"
                    font: addBtn.font
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                    opacity: addBtn.enabled ? 1.0 : 0.5
                }
            }
        }
    }

    Component.onCompleted: profileNameField.forceActiveFocus()

    onClosed: {
        profileNameField.text = ""
        addProfileDialog.profileName = ""
        addProfileDialog.pictureID = 1
    }
}
