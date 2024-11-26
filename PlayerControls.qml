import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Window

Window {
    id: controlsWindow
    flags: Qt.FramelessWindow | Qt.WindowStaysOnTopHint | Qt.Tool | Qt.NoDropShadowWindowHint

    color: "transparent"
    width: 600
    height: 200
    visible: true

    // Properties to sync with main player
    property bool isPlaying: false
    property int position: 0
    property int duration: 0
    property string title: ""
    property bool isDragging: false
    property Window parentWindow: null

    // Signals
    signal playPauseClicked()
    signal positionRequested(int position)
    signal closeRequested()

    // Invisible outer frame to prevent white border
    Rectangle {
        anchors {
            fill: parent
            margins: -1  // Extend slightly beyond window bounds
        }
        color: "transparent"
        border.width: 0

        // Main controls background
        Rectangle {
            id: controlsBackground
            anchors {
                fill: parent
                margins: 1  // Pull back to actual window bounds
            }
            color: "#CC000000"
            radius: 8
            border.width: 0

            ColumnLayout {
                anchors {
                    fill: parent
                    margins: 16
                }
                spacing: 8

                // Title
                Text {
                    text: title
                    color: "white"
                    font.pixelSize: 16
                    font.bold: true
                    elide: Text.ElideRight
                    Layout.fillWidth: true
                }

                // Progress bar row
                RowLayout {
                    Layout.fillWidth: true
                    spacing: 8

                    Text {
                        text: formatTime(position)
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
                                width: duration > 0 ? (position / duration) * parent.width : 0
                                height: parent.height
                                color: mouseArea.pressed ? "#1ED760" : "#1DB954"
                                radius: 2
                            }
                        }

                        Rectangle {
                            id: handle
                            x: duration > 0 ? (position / duration) * (parent.width - width) : 0
                            anchors.verticalCenter: parent.verticalCenter
                            width: mouseArea.pressed ? 20 : 16
                            height: width
                            radius: width / 2
                            color: mouseArea.pressed ? "#1ED760" : "#1DB954"

                            Behavior on width {
                                NumberAnimation { duration: 100 }
                            }
                        }

                        MouseArea {
                            id: mouseArea
                            anchors.fill: parent
                            hoverEnabled: true

                            onMouseXChanged: {
                                if (pressed) {
                                    console.log("Mouse dragged:", mouseX)
                                    let newPosition = (mouseX / width) * duration
                                    position = Math.max(0, Math.min(newPosition, duration))
                                }
                            }

                            onPressed: {
                                console.log("Mouse pressed:", mouseX)
                                let newPosition = (mouseX / width) * duration
                                position = Math.max(0, Math.min(newPosition, duration))
                                positionRequested(position)
                            }

                            onReleased: {
                                console.log("Mouse released:", mouseX)
                                positionRequested(position)
                            }
                        }
                    }

                    Text {
                        text: formatTime(duration)
                        color: "white"
                        font.pixelSize: 12
                    }
                }

                // Control buttons row
                RowLayout {
                    Layout.fillWidth: true
                    spacing: 16

                    Button {
                        text: "←"
                        onClicked: closeRequested()
                        flat: true
                        background: Rectangle {
                            color: "transparent"
                        }
                        contentItem: Text {
                            text: parent.text
                            color: "white"
                            font.pixelSize: 24
                        }
                    }

                    Button {
                        text: isPlaying ? "⏸" : "▶"
                        onClicked: playPauseClicked()
                        flat: true
                        background: Rectangle {
                            color: "transparent"
                        }
                        contentItem: Text {
                            text: parent.text
                            color: "white"
                            font.pixelSize: 24
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

                        onActivated: {
                            let trackId = currentValue
                            mediaPlayer.setAudioTrack(trackId)
                        }
                    }

                    ComboBox {
                        id: subtitleSelector
                        Layout.preferredWidth: 150
                        model: mediaPlayer.subtitleTracks
                        textRole: "name"
                        valueRole: "id"
                        currentIndex: 0

                        onActivated: {
                            
                            let trackId = currentValue  // This will be the track ID
                            if (trackId === -1) {
                                mediaPlayer.disableSubtitles()
                            } else {
                                mediaPlayer.setSubtitleTrack(trackId)
                            }
                        }
                    }

                    
                }
            }
        }
    }

    function formatTime(ms) {
        let seconds = Math.floor(ms / 1000)
        let minutes = Math.floor(seconds / 60)
        seconds = seconds % 60
        return minutes.toString().padStart(2, '0') + ':' + 
               seconds.toString().padStart(2, '0')
    }

    function updatePosition() {
        if (parentWindow) {
            x = parentWindow.x + (parentWindow.width - width) / 2
            y = parentWindow.y + parentWindow.height - height - 40
        }
    }

    Component.onCompleted: {
        updatePosition()
    }

    Connections {
        target: parentWindow
        function onXChanged() { updatePosition() }
        function onYChanged() { updatePosition() }
        function onWidthChanged() { updatePosition() }
        function onHeightChanged() { updatePosition() }
        function onVisibilityChanged() { 
            controlsWindow.visible = parentWindow.visible
            if (visible) {
                updatePosition()
                controlsWindow.raise()
            }
        }
    }
}