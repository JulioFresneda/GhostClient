import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtMultimedia
import com.ghoststream 1.0

Rectangle {
    id: root
    color: "black"

    required property string mediaId
    required property string title
    required property var mediaMetadata

    property bool isLoading: true
    property real loadingPos: -1
    property int loadingPosCounter: 0
    signal closeRequested
    signal mediaEnded

    function loadingPosFun(){
        if (isLoading && loadingPos < mediaPlayer.position){
            loadingPos = mediaPlayer.position
            loadingPosCounter += 1
        }
        if (loadingPosCounter == 3){
            isLoading = false
        }
    }

    Window {
        id: floatingWindow
        visible: isLoading
        flags: Qt.FramelessWindowHint | Qt.Window
        color: "transparent" // Set transparent background for the window
        width: root.width
        height: root.height

        Timer {
            id: periodicTimer
            interval: 1000 // Call every 1000 ms (1 second)
            running: true
            repeat: true
            onTriggered: {
                console.log("TIMER")
                root.loadingPosFun()
            }
        }


        Rectangle {
            id: loadingScreen
            color: "black"
            anchors.fill: parent

            Image {
                id: loadingImage
                source: "qrc:/media/loading.png" // Path to your loading image
                anchors.centerIn: parent
                scale: 0.5
                transform: Rotation {
                    id: rotation
                    origin.x: loadingImage.width / 2
                    origin.y: loadingImage.height / 2
                    angle: 0
                }
                
            }

            RotationAnimation {
                id: rotateAnimation
                target: rotation
                property: "angle"
                from: 0
                to: 360
                duration: 3000 // 3 seconds
                loops: Animation.Infinite
                running: false
            }

            Timer {
                id: startDelayTimer
                interval: 100 // 3 seconds delay
                running: true
                repeat: false
                onTriggered: {
                    rotateAnimation.start(); // Start the animation after the timer ends
                }
            }
        }


        // Ensure the window stays in sync with the parent
        Component.onCompleted: {
            let globalPos = root.mapToGlobal(Qt.point(0, 0));
            x = globalPos.x;
            y = globalPos.y;
            root.widthChanged.connect(() => width = root.width);
            root.heightChanged.connect(() => height = root.height);
            //isLoading = false
            
        }
    }

    // Create a basic column layout
    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        // Video area
        Rectangle {
            Layout.fillWidth: true
            // Use Layout.preferredHeight to make it take remaining space minus controls
            Layout.preferredHeight: parent.height - 160
            color: "black"

            VideoOutput {
                id: videoOutput
                anchors.fill: parent
                visible: false
            }

            VLCPlayerHandler {
                id: mediaPlayer
                videoSink: videoOutput.videoSink
                
                Component.onCompleted: {
                    if (root.mediaId) {
                        if (root.mediaMetadata !== undefined) {
                            loadMedia(root.mediaId, root.mediaMetadata)
                        } else {
                            loadMedia(root.mediaId, {})
                        }
                    }
                }
                onMediaEnded: {
                    root.mediaEnded()
                    if (root.mediaMetadata !== undefined) {
                        loadMedia(root.mediaId, root.mediaMetadata)
                    } else {
                        loadMedia(root.mediaId, {})
                    }
                }
            }
            
        }

        // Controls area with fixed height
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 160
            color: "#CC000000"

            ColumnLayout {
                anchors {
                    fill: parent
                    margins: 16
                }
                spacing: 8

                // Title
                Text {
                    text: root.title
                    color: "white"
                    font.pixelSize: 16
                    font.bold: true
                    elide: Text.ElideRight
                    Layout.fillWidth: true
                }

                // Progress bar
                RowLayout {
                    Layout.fillWidth: true
                    spacing: 8

                    Text {
                        text: formatTime(mediaPlayer.position)
                        color: "white"
                        font.pixelSize: 12
                    }

                    Item {
                        Layout.fillWidth: true
                        height: 20

                        Rectangle {
                            id: progressBar
                            anchors.verticalCenter: parent.verticalCenter
                            width: parent.width
                            height: 4
                            color: "#808080"
                            radius: 2

                            Rectangle {
                                width: mediaPlayer.duration > 0 ? 
                                       (mediaPlayer.position / mediaPlayer.duration) * parent.width : 0
                                height: parent.height
                                color: progressMouseArea.pressed ? "#1ED760" : "#1DB954"
                                radius: 2
                            }
                        }

                        Rectangle {
                            id: handle
                            x: mediaPlayer.duration > 0 ? 
                               (mediaPlayer.position / mediaPlayer.duration) * (parent.width - width) : 0
                            anchors.verticalCenter: parent.verticalCenter
                            width: progressMouseArea.pressed ? 20 : 16
                            height: width
                            radius: width / 2
                            color: progressMouseArea.pressed ? "#1ED760" : "#1DB954"
                        }

                        MouseArea {
                            id: progressMouseArea
                            anchors.fill: parent
                            hoverEnabled: true

                            onPressed: {
                                let newPosition = (mouseX / width) * mediaPlayer.duration
                                mediaPlayer.setPosition(Math.max(0, Math.min(newPosition, mediaPlayer.duration)))
                            }

                            onMouseXChanged: {
                                if (pressed) {
                                    let newPosition = (mouseX / width) * mediaPlayer.duration
                                    mediaPlayer.setPosition(Math.max(0, Math.min(newPosition, mediaPlayer.duration)))
                                }
                            }
                        }
                    }

                    Text {
                        text: formatTime(mediaPlayer.duration)
                        color: "white"
                        font.pixelSize: 12
                    }
                }

                // Controls
                RowLayout {
                    Layout.fillWidth: true
                    spacing: 16

                    Button {
                        text: "←"
                        onClicked: root.closeRequested()
                        flat: true
                        contentItem: Text {
                            text: parent.text
                            color: "white"
                            font.pixelSize: 24
                        }
                        background: Rectangle {
                            color: "transparent"
                        }
                    }

                    Button {
                        text: mediaPlayer.isPlaying ? "⏸" : "▶"
                        onClicked: mediaPlayer.isPlaying ? mediaPlayer.pauseMedia() : mediaPlayer.playMedia(0)
                        flat: true
                        contentItem: Text {
                            text: parent.text
                            color: "white"
                            font.pixelSize: 24
                        }
                        background: Rectangle {
                            color: "transparent"
                        }
                    }

                    Text {
                        text: "Audio:"
                        color: "white"
                        font.pixelSize: 12
                    }

                    ComboBox {
                        id: audioSelector
                        Layout.preferredWidth: 150
                        model: mediaPlayer.audioTracks
                        textRole: "name"
                        valueRole: "id"
                        currentIndex: 0
                        onActivated: mediaPlayer.setAudioTrack(currentValue)
                    }

                    ComboBox {
                        id: subtitleSelector
                        Layout.preferredWidth: 150
                        model: mediaPlayer.subtitleTracks
                        textRole: "name"
                        valueRole: "id"
                        currentIndex: 0
                        onActivated: {
                            let trackId = currentValue
                            if (trackId === -1) {
                                mediaPlayer.disableSubtitles()
                            } else {
                                mediaPlayer.setSubtitleTrack(trackId)
                            }
                        }
                    }

                    Button {
                        text: "Full screen"
                        onClicked: mediaPlayer.setFullScreen(true)
                        flat: true
                        contentItem: Text {
                            text: parent.text
                            color: "white"
                            font.pixelSize: 24
                        }
                        background: Rectangle {
                            color: "transparent"
                        }
                    }
                }
            }
        }
    }

    // Space bar shortcut
    Shortcut {
        sequence: "Space"
        onActivated: {
            if (mediaPlayer.isPlaying) {
                mediaPlayer.pauseMedia()
            } else {
                mediaPlayer.playMedia(0)
            }
        }
    }

    Component.onDestruction: {
        mediaPlayer.stop()
    }

    function formatTime(ms) {
        let seconds = Math.floor(ms / 1000)
        let minutes = Math.floor(seconds / 60)
        seconds = seconds % 60
        return minutes.toString().padStart(2, '0') + ':' + 
               seconds.toString().padStart(2, '0')
    }

    
}