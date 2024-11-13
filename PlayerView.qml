import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtMultimedia
import com.ghoststream 1.0

Item {
    id: root
    property string mediaId
    signal closePlayer

    Rectangle {
        anchors.fill: parent
        color: "black"

        VideoOutput {
            id: videoOutput
            anchors.fill: parent
        }

        VLCPlayerHandler {
            id: mediaPlayer
            videoSink: videoOutput.videoSink

            Component.onCompleted: {
                if (mediaId) {
                    loadMedia(mediaId)
                    playMedia()
                }
            }
        }

        // Controls overlay
        Rectangle {
            id: controlsOverlay
            anchors.bottom: parent.bottom
            width: parent.width
            height: 100
            color: "#80000000"  // Semi-transparent black

            RowLayout {
                anchors.fill: parent
                anchors.margins: 20

                Button {
                    text: mediaPlayer.isPlaying ? "⏸" : "▶"
                    onClicked: {
                        if (mediaPlayer.isPlaying) {
                            mediaPlayer.pauseMedia()
                        } else {
                            mediaPlayer.playMedia()
                        }
                    }
                }

                Slider {
                    Layout.fillWidth: true
                    from: 0
                    to: mediaPlayer.duration
                    value: mediaPlayer.position
                    onMoved: mediaPlayer.setPosition(value)
                }

                Button {
                    text: "×"
                    onClicked: {
                        mediaPlayer.stop()
                        closePlayer()
                    }
                }
            }
        }
    }
}