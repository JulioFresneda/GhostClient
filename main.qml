import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import com.ghoststream 1.0

ApplicationWindow {
    visible: true
    width: 1920
    height: 1080
    color: "#121212" // Dark background
    title: "Ghost Stream"

    Login {
        id: loginManager
    }
    Loader {
        id: startAnimation
        anchors.fill: parent
        source: "qrc:/qt/qml/ghostclient/StartAnimation.qml"
        active: true
        z: 100

        onLoaded: {
            item.animationFinished.connect(function() {
                splashFadeOut.start()
            })
        }
    }
    NumberAnimation {
        id: splashFadeOut
        target: startAnimation
        property: "opacity"
        from: 1
        to: 0
        duration: 500
        onFinished: {
            startAnimation.active = false
            profileView.opacity = 1
        }
    }

    Loader {
        id: appLoader
        anchors.fill: parent
        visible: false
        source: ""
        onStatusChanged: {
            if (status === Loader.Error) {
                console.error("Failed to load nav.qml")
            }
        }
    }

    // Profile selection view
    Rectangle {
        id: profileView
        visible: !appLoader.visible
        anchors.fill: parent
        color: "#121212"

        // Logo section
        Image {
            id: wallpaper
            source: "qrc:/media/wallpaper_login_1.png"
            anchors.fill: parent
        }

        

        // Profile grid container
        Item {
            id: profileContainer
            anchors {
                top: welcomeText.bottom
                topMargin: 60
                left: parent.left
                right: parent.right
            }
            height: parent.height*1.61

            Row {
                id: profileRow
                anchors.centerIn: parent
                spacing: 40
                
                // Existing profiles
                Repeater {
                    model: profileModel
                    delegate: Item {
                        width: 150
                        height: 150

                        Rectangle {
                            id: profileRect
                            width: 150
                            height: 150
                            //radius: 12
                            color: "#290D3D"
                            anchors.horizontalCenter: parent.horizontalCenter
                            scale: 1.0

                            Rectangle {
                                anchors.fill: parent
                                radius: parent.radius
                                gradient: Gradient {
                                    GradientStop { position: 0.0; color: "#419A38" }
                                    GradientStop { position: 0.5; color: "#163513" }
                                    GradientStop { position: 1.0; color: "#163513" }
                                }
                                visible: profileMouseArea.containsMouse
                            }

                            Image {
                                id: profileImage
                                source: "qrc:/media/ghosts/" + model.pictureID
                                anchors {
                                    fill: parent
                                    margins: 2
                                }
                                fillMode: Image.PreserveAspectFit
                            }

                            // Hover effect
                            

                            MouseArea {
                                id: profileMouseArea
                                anchors.fill: parent
                                hoverEnabled: true
                                onClicked: loginManager.selectProfile(model.profileID)
                                onEntered: profileRect.scale = 1.1
                                onExited: profileRect.scale = 1.0
                            }

                            Behavior on scale {
                                NumberAnimation {
                                    duration: 200
                                    easing.type: Easing.OutQuad
                                }
                            }
                        }

                        Rectangle {
                            anchors {
                                top: profileRect.bottom
                                topMargin: 15
                                horizontalCenter: profileRect.horizontalCenter
                            }
                            width: 150 // Add padding
                            height: contentText.height + 10 // Add padding
                            color: "#050505"  // Dark background color
                            radius: 0  // Slightly rounded corners
                            border.width: 2  // Border width
                            border.color: "#290D3D"

                            Text {
                                id: contentText
                                text: model.profileID
                                color: "#FFFFFF"
                                font {
                                    pointSize: 16
                                    weight: Font.Light
                                }
                                anchors.centerIn: parent
                                horizontalAlignment: Text.AlignHCenter
                            }
                        }
                    }
                }

                // Add profile button
                Item {
                    width: 200
                    height: 200
                    visible: profileModel.count < 5

                    Rectangle {
                        id: addProfileRect
                        width: 150
                        height: 150
                        radius: 12
                        color: addProfileMouseArea.containsMouse ? "#404040" : "#2A2A2A"
                        anchors.horizontalCenter: parent.horizontalCenter
                        scale: 1.0

                        Text {
                            text: "+"
                            color: "#FFFFFF"
                            font {
                                pointSize: 40
                                weight: Font.Light
                            }
                            anchors.centerIn: parent
                        }

                        MouseArea {
                            id: addProfileMouseArea
                            anchors.fill: parent
                            hoverEnabled: true
                            onClicked: addProfileDialog.open()
                            onEntered: addProfileRect.scale = 1.1
                            onExited: addProfileRect.scale = 1.0
                        }

                        Behavior on scale {
                            NumberAnimation {
                                duration: 200
                                easing.type: Easing.OutQuad
                            }
                        }
                    }

                    Text {
                        text: "Add Profile"
                        color: "#FFFFFF"
                        font {
                            pointSize: 16
                            weight: Font.Light
                        }
                        anchors {
                            top: addProfileRect.bottom
                            topMargin: 15
                            horizontalCenter: addProfileRect.horizontalCenter
                        }
                    }
                }
            }
        }
    }

    // Add Profile Dialog
    Dialog {
        id: addProfileDialog
        title: "Add New Profile"
        modal: true
        anchors.centerIn: parent
        width: 400
        height: 300
        background: Rectangle {
            color: "#2A2A2A"
            radius: 8
        }

        contentItem: ColumnLayout {
            spacing: 20

            Text {
                text: "Add Profile"
                color: "#FFFFFF"
                font.pointSize: 20
                Layout.alignment: Qt.AlignHCenter
            }

            TextField {
                id: profileNameField
                placeholderText: "Enter profile name"
                Layout.preferredWidth: 300
                Layout.alignment: Qt.AlignHCenter
                color: "#FFFFFF"
                background: Rectangle {
                    color: "#404040"
                    radius: 4
                }
            }

            ComboBox {
                id: avatarComboBox
                model: ["1", "2", "3", "4"]
                Layout.preferredWidth: 300
                Layout.alignment: Qt.AlignHCenter
            }

            RowLayout {
                Layout.alignment: Qt.AlignHCenter
                spacing: 10

                Button {
                    text: "Cancel"
                    onClicked: {
                        profileNameField.text = ""
                        addProfileDialog.close()
                    }
                }

                Button {
                    text: "Add"
                    enabled: profileNameField.text.length > 0
                    onClicked: {
                        loginManager.addProfile(profileNameField.text, avatarComboBox.currentText + ".png")
                        profileNameField.text = ""
                        addProfileDialog.close()
                    }
                }
            }
        }
    }

    ListModel {
        id: profileModel
    }

    Component.onCompleted: {
        loginManager.fetchUserProfile()
    }

    Connections {
        target: loginManager
        function onProfileDataFetched(profileData) {
            profileModel.clear()
            for (let i = 0; i < profileData.length; i++) {
                profileModel.append(profileData[i])
            }
        }
        function onProfileAdded() {
            profileModel.clear()
            loginManager.fetchUserProfile()
        }
        function onProfileSelected() {
            appLoader.source = "qrc:/qt/qml/ghostclient/nav.qml"
            appLoader.visible = true
            loginManager.fetchMediaData()
        }
    }
}