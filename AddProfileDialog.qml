import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Dialog {
    id: addProfileDialog
    modal: true
    visible: false
    width: 900
    height: 660
    x: (parent.width - width) / 2
    y: (parent.height - height) / 2
    dim: true


    property var profileName: ""
    property int pictureID: 1

    signal addProfileClicked()

    background: Rectangle {
        color: "#050505"
    }

    contentItem: Column {
        spacing: 20
        padding: 12
        Layout.alignment: Qt.AlignVCenter

        // Title
        Text {
            id: title
            font.pixelSize: 24
            text: "What is your ghost's name?"
            color: "#e2e2e2"
        }

        // Text Field
        TextField {
            id: profileNameField
            Layout.preferredWidth: 300
            width: 300
            Layout.preferredHeight: 40
            placeholderText: qsTr("Enter profile name")
            font.pixelSize: 24
            focus: true // Start with focus on the text field

            Keys.onReturnPressed: {
                imageGrid.forceActiveFocus()
            }
        }

        // GridView for profile images
        Rectangle {
            color: "#1E1E1E"
            height: 450
            width: 860
            Layout.alignment: Qt.AlignVCenter
            radius: 6

            GridView {
                id: imageGrid
                width: parent.width
                height: parent.height
                cellWidth: 140
                cellHeight: 140
                model: 36 // 36 images
                clip: true
                focus: false // Enable navigation focus

                delegate: Item {
                    width: 120
                    height: 120
                    focus: GridView.isCurrentItem
                    Rectangle {
                        color: "transparent"
                        anchors.fill: parent
                        border.width: 4
                        border.color: (index + 1) == pictureID && imageGrid.focus  ? "#419A38" : "#e2e2e2"
                    }

                    Image {
                        anchors.margins: 4
                        anchors.fill: parent
                        sourceSize.width: 120
                        sourceSize.height: 120
                        smooth: true
                        source: "qrc:/media/ghosts/" + (index + 1) + ".png"
                    }

                    MouseArea {
                        id: mouseArea
                        anchors.fill: parent
                        hoverEnabled: true
                        onClicked: {
                            addProfileDialog.pictureID = index + 1
                            console.log("Selected image: " + addProfileDialog.pictureID)
                            
                        }
                    }
                    Keys.onReturnPressed: {
                        console.log("Selected image: " + addProfileDialog.pictureID)
                        rowbuttons.forceActiveFocus()
                    }

                    Keys.onRightPressed: {
                        moveToNextItem(1)
                        //console.log(sidebar.usingit)
                    }
                    Keys.onLeftPressed: {
                        moveToPreviousItem(1)
                    }

                    Keys.onUpPressed: {
                        moveToPreviousItem(6)
                    }
                    Keys.onDownPressed: {
                        moveToNextItem(6)
                    }

                    function moveToNextItem(distance) {
                        if (addProfileDialog.pictureID < imageGrid.count - distance) {
                            addProfileDialog.pictureID += distance
                            console.log("Moved to next item:", addProfileDialog.pictureID)
                        }
                    }

                    function moveToPreviousItem(distance) {
                        if (addProfileDialog.pictureID > distance - 1) {
                            addProfileDialog.pictureID -= distance
                            console.log("Moved to previous item:", addProfileDialog.pictureID)
                        }
                    }

                }

                
            }
        }

        Item {
            Layout.fillHeight: true
        }

        // Buttons
        Row {
            spacing: 10
            Layout.alignment: Qt.AlignVCenter
            id: rowbuttons
            property bool addselected: true
            Keys.onReturnPressed: {
                if(addselected){
                    console.log("New profile name:", profileNameField.text)
                    profileName = profileNameField.text
                    addProfileClicked()
                    addProfileDialog.close()
                }
                else {
                    addProfileDialog.close()
                    profileName = ""
                    pictureID = 1
                }
            }
            Keys.onLeftPressed: {
                addselected = false
            }
            Keys.onRightPressed: {
                addselected = true
            }
            Button {
                text: qsTr("Cancel")
                //focus: !rowbuttons.addselected
                onClicked: {
                    addProfileDialog.close()
                    profileName = ""
                    pictureID = 1
                }
                
                font.pointSize: 16
                background: Rectangle {
                    color: "white"
                    border.color: !rowbuttons.addselected ? "#419A38" : "#050505"
                    border.width: 1
                    radius: 4
                }
            }

            Item {
                width: 40
            }

            Button {
                text: qsTr("Add Ghost")
                //focus: rowbuttons.addselected
                font.pointSize: 16
                onClicked: {
                    console.log("New profile name:", profileNameField.text)
                    profileName = profileNameField.text
                    addProfileClicked()
                    addProfileDialog.close()
                }
                
                horizontalPadding: 20
                background: Rectangle {
                    color: "white"
                    border.color: rowbuttons.addselected && rowbuttons.focus ? "#419A38" : "#e2e2e2"
                    border.width: 1
                    radius: 4
                }
            }
        }
    }

    Component.onCompleted: {
        profileNameField.forceActiveFocus() // Start with focus on the text field
    }
}

