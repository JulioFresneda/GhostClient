import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Dialog {
    id: addProfileDialog
    //title: qsTr("Add Profile")
    modal: true
    visible: false // Start hidden; call open() to display
    width: 900
    height: 660
    x: (parent.width  - width ) / 2
    y: (parent.height - height) / 2
    dim: true         // Dims everything behind the dialog
    //dimColor: "rgba(0, 0, 0, 0.6)" 
    Overlay.modal: Rectangle {
        color: "#80050505"  // Use whatever color/opacity you like
    }
    property var profileName: ""
    property var pictureID: 1

    signal addProfileClicked()

    background: Rectangle {
        // Change color here
        color: "#050505"
        //border.color: "#e2e2e2"
        
        //radius: 6 // optional corner rounding
    }
    
    contentItem: Column {
        spacing: 20
        padding: 12
        Layout.alignment: Qt.AlignVCenter
        Text {
            id: title
            font.pixelSize: 24
            text: "What is your ghost's name?"
            color: "#e2e2e2"
        }
        TextField {
            id: profileNameField
            Layout.preferredWidth: 300
            width: 300
            Layout.preferredHeight: 40
            placeholderText: qsTr("Enter profile name")
            font.pixelSize: 24
        }

        Rectangle {
            // Change color here
            color: "#1E1E1E"
            //border.color: "#e2e2e2"
            //border.round: 4
            height: 450
            width: 860
            Layout.alignment: Qt.AlignVCenter
            Layout.fillWidth: true
            Layout.fillHeight: true 
            anchors.margins: 10
            radius: 6
        // A GridView showing 4 images from qrc:/media/ghosts/
            GridView {
                
                id: imageGrid
                width: parent.width
                Layout.alignment: Qt.AlignVCenter
                height: parent.height
                cellWidth: 140
                cellHeight: 140
                // We'll show 4 images: 1.png, 2.png, 3.png, 4.png
                model: 36
                //Layout.fillWidth: true
                //Layout.fillHeight: true 
                clip: true
                
                    //border.width: 4

                delegate: Item {
                    width: 120
                    height: 120

                    Rectangle {
                        color: "transparent" 
                        anchors.fill: parent
                        border.width: 4
                        border.color: (index + 1) == pictureID ? "#419A38" : "#e2e2e2" 
                        //radius: 8 // optional rounded corners
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
                        id: profileMouseArea
                        anchors.fill: parent
                        hoverEnabled: true
                        onClicked: {
                            console.log("Selected image: " + (index + 1))
                            pictureID = (index + 1)
                            // You might store this “selected image” in a property
                        }
                    }
                }
            }
        }

        Item {
            Layout.fillHeight: true
        }

        // Buttons to confirm or cancel
        Row {
            spacing: 10
            Layout.alignment: Qt.AlignVCenter

            Button {
                text: qsTr("Cancel")
                onClicked: {
                    addProfileDialog.close()
                    profileName = ""
                    pictureID = 1
                }
                font.pointSize: 16
                background: Rectangle {
                    // Button background color
                    color: "#050505"
                    // Button border
                    //border.color: "#e2e2e2"
                    //border.width: 1
                    //radius: 4  // optional corner rounding
                }
            }
            Item {
                width: 40    
            }
            Button {
                text: qsTr("Add Ghost")
                font.pointSize: 16
                onClicked: {
                    console.log("New profile name:", profileNameField.text)
                    profileName = profileNameField.text
                    addProfileClicked()
                    addProfileDialog.close()
                }
                horizontalPadding: 20
                background: Rectangle {
                    color: "#050505"
                    border.color: "#e2e2e2"
                    border.width: 1
                    radius: 4
                }
            }
        }

    }
}
