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