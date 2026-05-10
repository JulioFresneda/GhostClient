import QtQuick
import QtQuick.Controls 
import QtQuick.Layouts
import com.ghoststream 1.0

/**
 * Main application window for Ghost Stream.
 * Sets up the UI structure, animations, and user interactions.
 */
ApplicationWindow {
    visible: true
    width: 1920
    height: 1080
    color: "#121212" // Dark background
    title: "Ghost Stream"
    visibility: Window.FullScreen

    /**
     * Centralized color scheme for the application.
     */
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

    /**
     * Manages user authentication and profiles.
     */
    Medium {
        id: loginManager
    }

    /**
     * Server Connection Status Indicator
     */
    Rectangle {
        id: connectionStatusBox
        // Only show on the profile-selector screen — not during the splash and
        // not after a profile has been chosen and the navigator is loaded.
        visible: profileView.visible && !startAnimation.active
        anchors {
            top: parent.top
            right: parent.right
            topMargin: 20
            rightMargin: 20
        }
        width: statusRow.width + 30
        height: 40
        color: colors.surface
        radius: 0
        border.color: loginManager.isConnected ? colors.superGreen : "#FF4444"
        border.width: 2
        opacity: 0.9
        z: 200

        Row {
            id: statusRow
            anchors.centerIn: parent
            spacing: 10

            Rectangle {
                width: 12
                height: 12
                radius: 6
                color: loginManager.isConnected ? colors.superGreen : "#FF4444"
                anchors.verticalCenter: parent.verticalCenter
                
                Rectangle {
                    anchors.fill: parent
                    radius: parent.radius
                    color: parent.color
                    opacity: 0.5
                    scale: 1.5
                }
            }

            Text {
                text: loginManager.isConnected ? "GhostServer ON" : "GhostServer OFF"
                color: colors.strongWhite
                font.pointSize: 12
                font.weight: Font.Medium
                anchors.verticalCenter: parent.verticalCenter
            }
        }

        Timer {
            interval: 10000
            running: true
            repeat: true
            onTriggered: loginManager.checkConnection()
        }
        
        Behavior on border.color {
            ColorAnimation { duration: 300 }
        }
    }

    /**
     * Exit button — top-left, mirrors the status indicator's positioning and
     * shares its visibility rules (profile selector only).
     */
    Rectangle {
        id: exitButton
        visible: profileView.visible && !startAnimation.active
        anchors {
            top: parent.top
            left: parent.left
            topMargin: 20
            leftMargin: 20
        }
        width: 40
        height: 40
        color: colors.surface
        radius: 0
        border.color: exitMouseArea.containsMouse ? "#FF4444" : "#33FFFFFF"
        border.width: 2
        opacity: 0.9
        z: 200

        Text {
            anchors.centerIn: parent
            text: "×"
            color: exitMouseArea.containsMouse ? "#FF6666" : colors.strongWhite
            font.pixelSize: 22
            font.weight: Font.Light
            Behavior on color { ColorAnimation { duration: 150 } }
        }

        MouseArea {
            id: exitMouseArea
            anchors.fill: parent
            hoverEnabled: true
            cursorShape: Qt.PointingHandCursor
            onClicked: Qt.quit()
        }

        Behavior on border.color { ColorAnimation { duration: 150 } }
    }

    /**
     * Splash screen animation loader.
     */
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

    /**
     * Fade-out animation for splash screen.
     */
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

    /**
     * Loader for the main application content.
     */
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
            appLoader.item.token = loginManager.getToken();
            appLoader.item.backToProfileSelect.connect(function() {
                appLoader.visible = false
                appLoader.source = ""
                profileRow.currentIndex = -1
                loginManager.fetchUserProfile()
            })
        }
    }

    /**
     * Dialog for adding user profiles.
     */
    AddProfileDialog {
        id: addProfileDialog
        onAddProfileClicked: {
            loginManager.addProfile(addProfileDialog.profileName, addProfileDialog.pictureID)
        }   
    }

    /**
     * View for selecting user profiles.
     */
    Rectangle {
        id: profileView
        visible: !appLoader.visible
        anchors.fill: parent
        color: "#0A0A0A"

        // Background — PreserveAspectCrop so the wallpaper fills any aspect
        // ratio without distortion (crops the long edge instead of stretching).
        Image {
            id: wallpaper
            source: "qrc:/media/wallpaper_login_1.png"
            anchors.fill: parent
            fillMode: Image.PreserveAspectCrop
            asynchronous: true
        }

        // ─────────────── Profile selector row ───────────────
        Item {
            id: profileContainer
            anchors.fill: parent

            Row {
                id: profileRow
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.bottom: parent.bottom
                anchors.bottomMargin: parent.height * 0.07
                spacing: 56

                // -1 = nothing pre-selected. Right-arrow lands on index 0,
                // hover/click activates the slot under the cursor.
                property int currentIndex: -1

                Repeater {
                    model: profileModel
                    delegate: ProfileSlot {
                        property bool active: index === profileRow.currentIndex || hoverArea.containsMouse
                        avatarSource: "qrc:/media/ghosts/" + model.pictureID
                        label: model.profileID
                        focused: active
                        onActivated: loginManager.selectProfile(model.profileID)
                    }
                }

                ProfileSlot {
                    id: addSlot
                    visible: profileModel.count < 5
                    property bool active: profileRow.currentIndex === profileModel.count || hoverArea.containsMouse
                    placeholder: true
                    focused: active
                    onActivated: addProfileDialog.open()
                }
            }

            Keys.onLeftPressed: {
                if (profileRow.currentIndex > 0) {
                    profileRow.currentIndex--
                    forceActiveFocus()
                }
            }
            Keys.onRightPressed: {
                var maxIndex = profileModel.count + (profileModel.count < 5 ? 1 : 0) - 1
                if (profileRow.currentIndex < maxIndex) {
                    profileRow.currentIndex++
                    forceActiveFocus()
                }
            }
            Keys.onReturnPressed: {
                if (profileRow.currentIndex < 0) return
                if (profileRow.currentIndex === profileModel.count) {
                    addProfileDialog.open()
                } else {
                    loginManager.selectProfile(profileModel.get(profileRow.currentIndex).profileID)
                }
            }

            Component.onCompleted: profileContainer.forceActiveFocus()
        }
    }

    /**
     * Data model for storing profile information.
     */
    ListModel {
        id: profileModel
    }

    /**
     * Fetch user profiles on application load.
     */
    Component.onCompleted: {
        loginManager.fetchUserProfile()
    }

    /**
     * Connections for handling profile-related signals.
     */
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