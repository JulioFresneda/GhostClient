import QtQuick
import QtQuick.Controls 
import QtQuick.Layouts
import com.ghoststream 1.0

ApplicationWindow {
    visible: true
    width: 1920
    height: 1080
    color: "#121212" // Dark background
    title: "Ghost Stream"
    visibility: Window.FullScreen

    QtObject {
        id: colors
        property string background: "#050505"
        property string surface: "#1E1E1E"
        property string primary: "#FCF7F8"
        property string textPrimary: "#050505"
        property string textSecondary: "#FCF7F8"
        property string strongViolet: "#290D3D"
        property string strongWhite: "#e2e2e2"
        property string green: "#419A38"
        property string superGreen: "#66f557"
    }

    Medium {
        id: loginManager
    }
    Loader {
        id: startAnimation
        anchors.fill: parent
        source: "qrc:/qt/qml/ghostclient/StartAnimation.qml"
        active: false
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
        onLoaded: {
            // Pass the token to the loaded item
            appLoader.item.token = loginManager.getToken();
        }
    }

    AddProfileDialog {
        id: addProfileDialog
        onAddProfileClicked: {
            loginManager.addProfile(addProfileDialog.profileName, addProfileDialog.pictureID)
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

            // KeyNavigation attached property for the container
            //Keys.onLeftPressed: profileRow.decrementCurrentIndex()
            //Keys.onRightPressed: profileRow.incrementCurrentIndex()
            Keys.forwardTo: profileContainer

            //Component.onCompleted: {
            //    profileContainer.forceActiveFocus() // Ensure focus starts at this container
            //}

            Row {
                id: profileRow
                anchors.centerIn: parent
                spacing: 40
        
                // Property to track current focus index
                property int currentIndex: 0
        
                
        
                // Existing profiles
                Repeater {
                    model: profileModel
                    delegate: Item {
                        id: profileItem
                        width: 160
                        height: 160

                        // Make item focusable
                        focus: index === profileRow.currentIndex
                      
                
                        Rectangle {
                            id: profileRect
                            width: 160
                            height: 160
                            anchors.horizontalCenter: parent.horizontalCenter
                            scale: index === profileRow.currentIndex || profileMouseArea.containsMouse ? 1.1 : 1.0
                            color: colors.strongWhite

                            // Focus visual indicator
                            Rectangle {
                                anchors.fill: parent
                                radius: parent.radius
                                gradient: Gradient {
                                    GradientStop { position: 0.0; color: "#419A38" }
                                    GradientStop { position: 0.5; color: "#163513" }
                                    GradientStop { position: 1.0; color: "#163513" }
                                }
                                visible: index === profileRow.currentIndex || profileMouseArea.containsMouse
                            }

                            

                            Image {
                                id: profileImage
                                source: "qrc:/media/ghosts/" + model.pictureID
                                anchors {
                                    fill: parent
                                    margins: 5
                                }
                                sourceSize.width: 160
                                sourceSize.height: 160
                            }

                            Rectangle {
                                anchors {
                                    top: profileRect.bottom
                                    topMargin: -5
                                    horizontalCenter: profileRect.horizontalCenter
                                }
                                width: 60
                                height: contentText.height + 10
                                color: "transparent"
                                bottomLeftRadius: 50
                                bottomRightRadius: 50

                                Text {
                                    id: contentText
                                    text: model.profileID
                                    color: "white"
                                    font {
                                        pointSize: 16
                                    }
                                    anchors.centerIn: parent
                                    horizontalAlignment: Text.AlignHCenter
                                    elide: Text.ElideRight
                                    wrapMode: Text.WordWrap
                                }
                            }

                            MouseArea {
                                id: profileMouseArea
                                anchors.fill: parent
                                hoverEnabled: true
                                onClicked: loginManager.selectProfile(model.profileID)
                            }

                            Behavior on scale {
                                NumberAnimation {
                                    duration: 200
                                    easing.type: Easing.OutQuad
                                }
                            }
                        }

                        // Handle Enter/Return key press
                        //Keys.onReturnPressed: loginManager.selectProfile(model.profileID)
                        //Keys.onEnterPressed: loginManager.selectProfile(model.profileID)

                        // Set focus when current index matches
                        //onActiveFocusChanged: {
                        //    if (activeFocus) {
                        //        profileRow.currentIndex = index
                        //    }
                        //}

                        //Component.onCompleted: {
                        //    if (index === 0) {
                        //        profileItem.forceActiveFocus()
                        //    }
                        //}
                    }
                }

                // Add profile button
                Item {
                    id: addProfileItem
                    width: 160
                    height: 160
                    visible: profileModel.count < 5
            
                    // Make add button focusable
                    //activeFocusOnTab: true
                    ////KeyNavigation.left: profileRepeater.count > 0 ? profileRepeater.itemAt(profileRepeater.count - 1) : null

                    Rectangle {
                        id: addProfileRect
                        width: 160
                        height: 160
                        color: colors.background
                        border.width: 5
                        border.color: colors.strongWhite
                        anchors.centerIn: parent
                        scale: profileRow.currentIndex === profileModel.count || addProfileMouseArea.containsMouse ? 1.1 : 1.0

                        Rectangle {
                            anchors.fill: parent
                            radius: parent.radius
                            gradient: Gradient {
                                GradientStop { position: 0.0; color: "#419A38" }
                                GradientStop { position: 1.0; color: "#163513" }
                            }
                            visible: profileRow.currentIndex === profileModel.count || addProfileMouseArea.containsMouse
                        }

                        

                        Text {
                            text: "+"
                            color: colors.strongWhite
                            font {
                                pointSize: 180
                                weight: Font.Light
                            }
                            x: (parent.width  - width ) / 2
                            y: (parent.height - height) / 2 - 27
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                        }

                        MouseArea {
                            id: addProfileMouseArea
                            anchors.fill: parent
                            hoverEnabled: true
                            onClicked: addProfileDialog.open()
                        }

                        Behavior on scale {
                            NumberAnimation {
                                duration: 200
                                easing.type: Easing.OutQuad
                            }
                        }
                    }

                    Text {
                        text: "Add Ghost"
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
        
            Keys.onLeftPressed: {
                if (profileRow.currentIndex > 0) {
                    profileRow.currentIndex--
                    forceActiveFocus() // Ensure container retains focus
                }
            }

            Keys.onRightPressed: {
                var maxIndex = profileModel.count + (profileModel.count < 5 ? 1 : 0) - 1
                if (profileRow.currentIndex < maxIndex) {
                    profileRow.currentIndex++
                    forceActiveFocus() // Ensure container retains focus
                }
            }

            Keys.onReturnPressed: {
                if (profileRow.currentIndex === profileModel.count) {
                    addProfileDialog.open()
                } else {
                    loginManager.selectProfile(profileModel.get(profileRow.currentIndex).profileID)
                }
            }

            Component.onCompleted: {
                profileContainer.forceActiveFocus() // Ensure focus starts at this container
            }
        }
    }

    // Add Profile Dialog
    

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
            appLoader.source = "qrc:/qt/qml/ghostclient/Navigator.qml"
            appLoader.visible = true
            loginManager.fetchMediaData()
        }
    }
}