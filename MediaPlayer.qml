// MediaPlayer.qml
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtMultimedia
import com.ghoststream

Item {
    id: root
    
    required property string mediaId
    required property string title
    signal closeRequested

    // Main layout
    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        // Video output area
        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            color: "black"

            // Video output
            VideoOutput {
                id: videoOutput
                anchors.fill: parent
            }

            // Media player
            MediaPlayer {
                id: player
                mediaId: root.mediaId
                title: root.title
                videoSink: videoOutput.videoSink

                onError: function(message) {
                    console.error("Player error:", message)
                }

                onCloseRequested: {
                    root.closeRequested()
                }
            }

            // Close button
            Button {
                anchors {
                    top: parent.top
                    right: parent.right
                    margins: 10
                }
                text: "×"
                onClicked: player.closePlayer()
            }
        }

        // Controls area
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 60
            color: "#1a1a1a"

            RowLayout {
                anchors {
                    fill: parent
                    margins: 10
                }
                spacing: 10

                Button {
                    text: "Play"
                    onClicked: player.play()
                }

                Button {
                    text: "Pause"
                    onClicked: player.pause()
                }

                Button {
                    text: "Stop"
                    onClicked: player.stop()
                }

                ComboBox {
                    Layout.preferredWidth: 150
                    model: ["No Subtitles", "English", "Spanish"]
                    onCurrentIndexChanged: {
                        if (currentIndex === 0) {
                            player.disableSubtitles()
                        } else {
                            player.setSubtitles(currentIndex - 1)
                        }
                    }
                }

                Item { Layout.fillWidth: true } // Spacer
            }
        }
    }
}