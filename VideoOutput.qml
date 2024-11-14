import QtQuick
import QtQuick.Controls
import QtMultimedia
import com.ghoststream 1.0

Rectangle {
    id: videoRoot
    color: "black"
    
    required property string mediaId
    required property string title
    
    signal playStateChanged(bool isPlaying)
    signal positionChanged(real position)
    signal durationChanged(real duration)
    
    VideoOutput {
        id: videoOutput
        anchors.fill: parent
    }
    
    VLCPlayerHandler {
        id: mediaPlayer
        videoSink: videoOutput.videoSink
        
        onPlayingStateChanged: videoRoot.playStateChanged(isPlaying)
        onPositionChanged: videoRoot.positionChanged(position)
        onDurationChanged: videoRoot.durationChanged(duration)
        
        Component.onCompleted: {
            if (videoRoot.mediaId) {
                loadMedia(videoRoot.mediaId)
                playMedia()
            }
        }
    }
    
    // Expose methods to control the video
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
    
    Component.onDestruction: {
        stop()
    }
}