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

                    Slider {
                        id: progressSlider
                        Layout.fillWidth: true
                        from: 0
                        to: Math.max(1, duration)
                        value: isDragging ? value : position
                        
                        onPressedChanged: {
                            if (!pressed && value !== position) {
                                isDragging = false
                                positionRequested(value)
                            } else {
                                isDragging = pressed
                            }
                        }

                        background: Rectangle {
                            x: progressSlider.leftPadding
                            y: progressSlider.topPadding + progressSlider.availableHeight / 2 - height / 2
                            width: progressSlider.availableWidth
                            height: 4
                            radius: 2
                            color: "#808080"

                            Rectangle {
                                width: progressSlider.visualPosition * parent.width
                                height: parent.height
                                color: "#1DB954"
                                radius: 2
                            }
                        }

                        handle: Rectangle {
                            x: progressSlider.leftPadding + progressSlider.visualPosition 
                               * (progressSlider.availableWidth - width)
                            y: progressSlider.topPadding + progressSlider.availableHeight / 2 - height / 2
                            width: 16
                            height: 16
                            radius: 8
                            color: "#1DB954"
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