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

    signal closeRequested

    // Remove MediaPlayer component since we don't need it
    // We'll use VLCPlayerHandler directly

    // Main video layer
    Item {
        anchors.fill: parent

        Item {
            id: videoLayer
            anchors.fill: parent
            z: 1

            VideoOutput {
                id: videoOutput
                anchors.fill: parent
                z: 1
            }

            VLCPlayerHandler {
                id: mediaPlayer
                videoSink: videoOutput.videoSink

                Component.onCompleted: {
                    if (root.mediaId) {
                        console.log(root.mediaMetadata)
                        if (root.mediaMetadata !== undefined) { 
                            loadMedia(root.mediaId, root.mediaMetadata)
                        }
                        else {
                            loadMedia(root.mediaId, {})
                        }
                        if (root.mediaMetadata !== undefined) { 
                            //playMedia(root.mediaMetadata.percentage_watched)
                        }
                        else {
                            //playMedia(0)
                        }
                        
                    }
                }
            }
        }

        // Floating controls window loader
        Loader {
            id: controlsLoader
            active: false
            sourceComponent: Component {
                PlayerControls {
                    title: root.title
                    isPlaying: mediaPlayer.isPlaying
                    position: mediaPlayer.position
                    duration: mediaPlayer.duration
                    parentWindow: root.Window.window

                    onPlayPauseClicked: {
                        if (mediaPlayer.isPlaying) {
                            mediaPlayer.pauseMedia()
                        } else {
                            mediaPlayer.playMedia(0)
                        }
                    }

                    onPositionRequested: (newPosition) => {
                        mediaPlayer.setPosition(newPosition)
                    }

                    onCloseRequested: {
                        root.closeRequested()
                    }
                }
            }
        }
    }

    // Mouse area for showing/hiding controls
    MouseArea {
        id: mouseArea
        anchors.fill: parent
        hoverEnabled: true
        
        property bool cursorVisible: true
        
        onPositionChanged: {
            cursorTimer.restart()
            if (controlsLoader.item) {
                controlsLoader.item.visible = true
            }
            cursorVisible = true
        }

        Timer {
            id: cursorTimer
            interval: 3000
            onTriggered: {
                mouseArea.cursorVisible = false
                if (controlsLoader.item && mediaPlayer.isPlaying) {
                    controlsLoader.item.visible = true
                }
            }
        }
    }

    Component.onCompleted: {
        initControls()
    }

    Component.onDestruction: {
        mediaPlayer.stop()
        if (controlsLoader.item) {
            controlsLoader.item.close()
        }
    }

    function initControls() {
        if (!controlsLoader.active) {
            controlsLoader.active = true
            controlsLoader.item.visible = true
        }
    }
}