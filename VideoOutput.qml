import QtQuick
import QtQuick.Controls
import QtMultimedia
import com.ghoststream 1.0

Rectangle {
    id: root
    color: "black"

    // Properties moved to non-required
    property string mediaId: ""
    property string title: ""

    VideoOutput {
        id: videoOutput
        anchors.fill: parent
    }

    VLCPlayerHandler {
        id: mediaPlayer
        videoSink: videoOutput.videoSink

        Component.onCompleted: {
            if (root.mediaId) {
                loadMedia(root.mediaId)
                playMedia()
            }
        }
    }

    // Expose properties and methods for external use
    property alias isPlaying: mediaPlayer.isPlaying
    property alias position: mediaPlayer.position
    property alias duration: mediaPlayer.duration

    function play() {
        mediaPlayer.playMedia()
    }

    function pause() {
        mediaPlayer.pauseMedia()
    }

    function setPosition(pos) {
        mediaPlayer.setPosition(pos)
    }

    function stop() {
        mediaPlayer.stop()
    }
}