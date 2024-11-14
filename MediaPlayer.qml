import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtMultimedia
import com.ghoststream 1.0

Rectangle {
    id: root
    color: "black"

    

    // Add a test MouseArea over everything
    MouseArea {
        id: testArea
        anchors.fill: parent
        z: 1000 // Very high z to test if we can get any events
        hoverEnabled: true
        
        onEntered: console.log("Mouse ENTERED test area")
        onExited: console.log("Mouse EXITED test area")
        onPositionChanged: console.log("Mouse MOVED in test area:", mouseX, mouseY)
        onClicked: {
            console.log("Mouse CLICKED in test area")
            if (mediaPlayer.isPlaying) {
                mediaPlayer.pauseMedia()
            } else {
                mediaPlayer.playMedia()
            }
        }
    }

    required property string mediaId
    required property string title

    signal closeRequested

    MediaPlayer {
        id: mediaPlayerHandler
        mediaId: root.mediaId
        title: root.title
    }

    Item {
        anchors.fill: parent

        // Controls layer (underneath)
        Item {
            id: controlsLayer
            anchors.fill: parent
            z: 1

            Component.onCompleted: {
                console.log("=== Controls Layer Completed ===")
                console.log("ControlsLayer size:", width, "x", height)
            }

            Rectangle {
                anchors.fill: parent
                color: "transparent"

                Rectangle {
                    id: topControls
                    height: 60
                    width: parent.width
                    color: "#80000000"
                    opacity: controlsVisible ? 1 : 0

                    Behavior on opacity { NumberAnimation { duration: 200 } }

                    RowLayout {
                        anchors.fill: parent
                        anchors.margins: 10
                        spacing: 8

                        Button {
                            text: "← Back"
                            onClicked: root.closeRequested()
                            background: Rectangle {
                                color: "transparent"
                                border.color: "white"
                                border.width: 1
                                radius: 4
                            }
                            contentItem: Text {
                                text: parent.text
                                color: "white"
                                horizontalAlignment: Text.AlignHCenter
                                verticalAlignment: Text.AlignVCenter
                            }
                        }

                        Item { Layout.fillWidth: true }

                        Text {
                            text: root.title
                            color: "white"
                            font.pointSize: 14
                        }
                    }
                }

                Rectangle {
                    id: bottomControls
                    anchors.bottom: parent.bottom
                    width: parent.width
                    height: 80
                    color: "#80000000"
                    opacity: controlsVisible ? 1 : 0

                    Behavior on opacity { NumberAnimation { duration: 200 } }

                    RowLayout {
                        anchors.fill: parent
                        anchors.margins: 20
                        spacing: 20

                        Button {
                            text: mediaPlayer.isPlaying ? "⏸" : "▶"
                            onClicked: {
                                if (mediaPlayer.isPlaying) {
                                    mediaPlayer.pauseMedia()
                                } else {
                                    mediaPlayer.playMedia()
                                }
                            }
                            background: Rectangle {
                                color: "transparent"
                                border.color: "white"
                                border.width: 1
                                radius: 4
                            }
                            contentItem: Text {
                                text: parent.text
                                color: "white"
                                font.pointSize: 14
                                horizontalAlignment: Text.AlignHCenter
                                verticalAlignment: Text.AlignVCenter
                            }
                        }

                        Text {
                            text: formatTime(mediaPlayer.position) + " / " + formatTime(mediaPlayer.duration)
                            color: "white"
                            font.pointSize: 12
                        }

                        Slider {
                            Layout.fillWidth: true
                            from: 0
                            to: mediaPlayer.duration
                            value: mediaPlayer.position
                            onMoved: mediaPlayer.setPosition(value)

                            background: Rectangle {
                                x: parent.leftPadding
                                y: parent.topPadding + parent.availableHeight / 2 - height / 2
                                width: parent.availableWidth
                                height: 4
                                radius: 2
                                color: "#404040"

                                Rectangle {
                                    width: parent.width * (mediaPlayer.position / mediaPlayer.duration)
                                    height: parent.height
                                    color: "#1DB954"
                                    radius: 2
                                }
                            }

                            handle: Rectangle {
                                x: parent.leftPadding + parent.visualPosition * (parent.availableWidth - width)
                                y: parent.topPadding + parent.availableHeight / 2 - height / 2
                                width: 16
                                height: 16
                                radius: 8
                                color: "#1DB954"
                            }
                        }
                    }
                }
            }
        }

        // Video layer (on top)
        Item {
            id: videoLayer
            anchors.fill: parent
            z: 2

            Component.onCompleted: {
                console.log("=== Video Layer Completed ===")
                console.log("VideoLayer size:", width, "x", height)
            }

            VideoOutput {
                id: videoOutput
                anchors.fill: parent

                Component.onCompleted: {
                    console.log("=== VideoOutput Completed ===")
                    console.log("VideoOutput size:", width, "x", height)
                }
            }

            // Add inside the controls MouseArea
            MouseArea {
                anchors.fill: parent
                hoverEnabled: true
                z: 3
                onPositionChanged: {
                    console.log("=== Controls Debug ===")
                    console.log("Mouse position:", mouseX, mouseY)
                    console.log("Controls z-index:", controlsLayer.z)
                    console.log("Video z-index:", videoLayer.z)
                    console.log("Controls opacity:", topControls.opacity)
                    console.log("Video visible:", videoOutput.visible)
                    console.log("Video position:", videoOutput.x, videoOutput.y)
                    console.log("Video size:", videoOutput.width, videoOutput.height)
                    console.log("===================")
        
                    controlsTimer.restart()
                    topControls.opacity = 1
                    bottomControls.opacity = 1
                }
            
                onClicked: {
                    if (mediaPlayer.isPlaying) {
                        mediaPlayer.pauseMedia()
                    } else {
                        mediaPlayer.playMedia()
                    }
                }
            }

            VLCPlayerHandler {
                id: mediaPlayer
                videoSink: videoOutput.videoSink

                Component.onCompleted: {
                    console.log("=== VLCPlayer Completed ===")
                    if (root.mediaId) {
                        console.log("Loading media:", root.mediaId)
                        loadMedia(root.mediaId)
                        playMedia()
                    }
                }
            }
        }
    }

    // Control visibility state
    property bool controlsVisible: true

    Timer {
        id: controlsTimer
        interval: 3000
        onTriggered: controlsVisible = false
    }

    function formatTime(ms) {
        let seconds = Math.floor(ms / 1000)
        let minutes = Math.floor(seconds / 60)
        seconds = seconds % 60
        return minutes.toString().padStart(2, '0') + ':' + 
               seconds.toString().padStart(2, '0')
    }

    Component.onCompleted: {
        console.log("=== MediaPlayer Root Completed ===")
        console.log("Root size:", width, "x", height)
        if (mediaId) {
            mediaPlayer.loadMedia(mediaId)
            mediaPlayer.playMedia()
        }
    }

    Component.onDestruction: {
        mediaPlayer.stop()
    }
}