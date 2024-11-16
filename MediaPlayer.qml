import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Qt.labs.platform as Platform
import com.ghoststream 1.0

Rectangle {
    id: root
    color: "black"

    required property string mediaId
    required property string title

    signal closeRequested

    // Native window container for FFplay output
    Item {
        id: videoContainer
        anchors.fill: parent

        FFplayHandler {
            id: mediaPlayer
            videoWidget: videoContainer
            
            Component.onCompleted: {
                loadMedia(root.mediaId)
            }
        }
    }

    // Controls layers
    Item {
        anchors.fill: parent
        
        // Top controls bar
        Rectangle {
            id: topControls
            height: 60
            width: parent.width
            color: "#80000000"
            opacity: controlsVisible ? 1 : 0
            z: 2

            Behavior on opacity {
                NumberAnimation { duration: 200 }
            }

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

        // Bottom controls bar
        Rectangle {
            id: bottomControls
            anchors.bottom: parent.bottom
            width: parent.width
            height: 80
            color: "#80000000"
            opacity: controlsVisible ? 1 : 0
            z: 2

            Behavior on opacity {
                NumberAnimation { duration: 200 }
            }

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
                    text: formatTime(mediaPlayer.position)
                    color: "white"
                    font.pointSize: 12
                }

                Slider {
                    Layout.fillWidth: true
                    from: 0
                    to: 100
                    value: mediaPlayer.position / 1000
                    onMoved: mediaPlayer.setPosition(value * 1000)

                    background: Rectangle {
                        x: parent.leftPadding
                        y: parent.topPadding + parent.availableHeight / 2 - height / 2
                        width: parent.availableWidth
                        height: 4
                        radius: 2
                        color: "#404040"

                        Rectangle {
                            width: parent.width * (mediaPlayer.position / (100 * 1000))
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

    // Mouse area for showing/hiding controls
    MouseArea {
        anchors.fill: parent
        hoverEnabled: true
        z: 1
        
        onPositionChanged: {
            controlsTimer.restart()
            controlsVisible = true
        }
        
        onClicked: {
            if (mediaPlayer.isPlaying) {
                mediaPlayer.pauseMedia()
            } else {
                mediaPlayer.playMedia()
            }
        }
    }

    // Control visibility state
    property bool controlsVisible: true

    Timer {
        id: controlsTimer
        interval: 3000
        onTriggered: controlsVisible = false
        running: true
    }

    function formatTime(ms) {
        let seconds = Math.floor(ms / 1000)
        let minutes = Math.floor(seconds / 60)
        seconds = seconds % 60
        return minutes.toString().padStart(2, '0') + ':' + 
               seconds.toString().padStart(2, '0')
    }

    // Cleanup on destruction
    Component.onDestruction: {
        mediaPlayer.stop()
    }

    // Handle key events
    Keys.onSpacePressed: {
        if (mediaPlayer.isPlaying) {
            mediaPlayer.pauseMedia()
        } else {
            mediaPlayer.playMedia()
        }
    }

    Keys.onEscapePressed: {
        root.closeRequested()
    }

    Keys.onLeftPressed: {
        mediaPlayer.setPosition(Math.max(0, mediaPlayer.position - 10000))
    }

    Keys.onRightPressed: {
        mediaPlayer.setPosition(mediaPlayer.position + 10000)
    }

    focus: true
    onActiveFocusChanged: {
        if (activeFocus) {
            controlsVisible = true
            controlsTimer.restart()
        }
    }
}