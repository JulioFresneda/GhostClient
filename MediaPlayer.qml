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

    signal closeRequested

    MediaPlayer {
        id: mediaPlayerHandler
        mediaId: root.mediaId
        title: root.title
    }

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
                        loadMedia(root.mediaId)
                        playMedia()
                        initControls()
                    }
                }
            }
        }

        Loader {
            id: controlsLoader
            active: true
            sourceComponent: Component {
                PlayerControls {
                    title: root.title
                    isPlaying: mediaPlayer.isPlaying
                    position: mediaPlayer.position
                    duration: mediaPlayer.duration
                    parentWindow: root.Window.window
                    visible: true

                    onPlayPauseClicked: {
                        if (mediaPlayer.isPlaying) {
                            mediaPlayer.pauseMedia()
                        } else {
                            mediaPlayer.playMedia()
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

    // Mouse area for play/pause toggle on video click
    MouseArea {
        anchors.fill: parent
        onClicked: {
            if (mediaPlayer.isPlaying) {
                mediaPlayer.pauseMedia()
            } else {
                mediaPlayer.playMedia()
            }
        }
    }

    Component.onDestruction: {
        mediaPlayer.stop()
        if (controlsLoader.item) {
            controlsLoader.item.close()
        }
    }
}