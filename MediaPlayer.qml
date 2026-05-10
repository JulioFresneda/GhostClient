import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtMultimedia
import com.ghoststream 1.0
import QtQuick.VectorImage

Rectangle {
    id: root
    color: "black"

    required property string mediaId
    required property string title
    required property var mediaMetadata
    required property string episodeType

    property bool isLoading: true

    signal closeRequested
    signal mediaEnded
    signal nextEpisode
    signal lastEpisode
    signal updateMediaMetadata

    width: Screen.desktopAvailableWidth
    height: Screen.desktopAvailableHeight

    function restartLoadingWindow() { isLoading = true }

    function formatTime(ms) {
        let totalSeconds = Math.max(0, Math.floor(ms / 1000))
        let h = Math.floor(totalSeconds / 3600)
        let m = Math.floor((totalSeconds % 3600) / 60)
        let s = totalSeconds % 60
        if (h > 0) {
            return h + ":" + m.toString().padStart(2, '0') + ":" + s.toString().padStart(2, '0')
        }
        return m.toString().padStart(2, '0') + ":" + s.toString().padStart(2, '0')
    }

    Component.onDestruction: mediaPlayer.stop()

    // First frame → hide loading
    Connections {
        target: videoOutput.videoSink
        function onVideoFrameChanged() {
            if (root.isLoading) root.isLoading = false
        }
    }

    // ─────────────── Loading window ───────────────
    Window {
        id: loadingWindow
        visible: isLoading
        flags: Qt.FramelessWindowHint | Qt.Window
        color: "transparent"
        width: 1920
        height: 1080

        Rectangle {
            id: loadingScreen
            color: "black"
            anchors.fill: parent

            Image {
                id: loadingImage
                source: "qrc:/media/loading.png"
                anchors.centerIn: parent
                scale: 0.5

                transform: Rotation {
                    id: rotation
                    origin.x: loadingImage.width / 2
                    origin.y: loadingImage.height / 2
                    angle: 0
                }

                SequentialAnimation on opacity {
                    loops: Animation.Infinite
                    running: true
                    NumberAnimation { from: 0.2; to: 1.0; duration: 900; easing.type: Easing.InOutQuad }
                    NumberAnimation { from: 1.0; to: 0.2; duration: 900; easing.type: Easing.InOutQuad }
                }
            }

            RotationAnimation {
                target: rotation
                property: "angle"
                from: 0
                to: 360
                duration: 3000
                loops: Animation.Infinite
                running: true
            }
        }

        Component.onCompleted: {
            let globalPos = root.mapToGlobal(Qt.point(0, 0));
            x = globalPos.x;
            y = globalPos.y;
            root.widthChanged.connect(() => width = root.width);
            root.heightChanged.connect(() => height = root.height);
        }
    }

    // ─────────────── Video layer (full screen) ───────────────
    VideoOutput {
        id: videoOutput
        anchors.fill: parent
        visible: true
        fillMode: VideoOutput.PreserveAspectFit
    }

    VLCPlayerHandler {
        id: mediaPlayer

        Component.onCompleted: {
            if (root.mediaId) {
                videoSink = videoOutput.videoSink
                if (root.mediaMetadata !== undefined) {
                    loadMedia(root.mediaId, root.mediaMetadata)
                } else {
                    loadMedia(root.mediaId, {})
                }
            }
        }
        onMediaEnded: {
            root.mediaEnded()
            if (root.mediaMetadata !== undefined) {
                loadMedia(root.mediaId, root.mediaMetadata)
            } else {
                loadMedia(root.mediaId, {})
            }
        }
    }

    // ─────────────── Top bar (back + title) ───────────────
    Rectangle {
        id: topBar
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        height: 96
        opacity: mediaPlayer.fullScreen ? 0 : 1
        visible: opacity > 0
        Behavior on opacity { NumberAnimation { duration: 250; easing.type: Easing.InOutQuad } }
        gradient: Gradient {
            GradientStop { position: 0.0; color: "#CC000000" }
            GradientStop { position: 1.0; color: "#00000000" }
        }

        RowLayout {
            anchors.fill: parent
            anchors.leftMargin: 32
            anchors.rightMargin: 32
            anchors.topMargin: 14
            spacing: 20

            Button {
                id: backButton
                onClicked: {
                    mediaPlayer.updateMediaMetadata()
                    root.closeRequested()
                }
                flat: true
                Layout.preferredWidth: 28
                Layout.preferredHeight: 28
                Layout.alignment: Qt.AlignTop
                contentItem: VectorImage {
                    source: parent.hovered ? "qrc:/media/buttons/left_hover.svg" : "qrc:/media/buttons/left.svg"
                    anchors.fill: parent
                    preferredRendererType: VectorImage.CurveRenderer
                }
                background: Rectangle { color: "transparent" }
            }

            Text {
                text: root.title
                color: "white"
                font.pixelSize: 22
                font.bold: true
                elide: Text.ElideRight
                Layout.fillWidth: true
                Layout.alignment: Qt.AlignVCenter
            }
        }
    }

    // ─────────────── Bottom bar (progress + controls) ───────────────
    Item {
        id: bottomBar
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        height: 180
        opacity: mediaPlayer.fullScreen ? 0 : 1
        visible: opacity > 0
        Behavior on opacity { NumberAnimation { duration: 250; easing.type: Easing.InOutQuad } }

        Rectangle {
            anchors.fill: parent
            gradient: Gradient {
                GradientStop { position: 0.0; color: "#00000000" }
                GradientStop { position: 1.0; color: "#EE000000" }
            }
        }

        ColumnLayout {
            anchors.fill: parent
            anchors.leftMargin: 32
            anchors.rightMargin: 32
            anchors.topMargin: 36
            anchors.bottomMargin: 24
            spacing: 18

            // ──── Progress row ────
            RowLayout {
                Layout.fillWidth: true
                spacing: 14

                Text {
                    text: formatTime(mediaPlayer.position)
                    color: "white"
                    font.pixelSize: 13
                    Layout.preferredWidth: 60
                    horizontalAlignment: Text.AlignRight
                }

                Item {
                    Layout.fillWidth: true
                    height: 16

                    Rectangle {
                        id: progressTrack
                        anchors.verticalCenter: parent.verticalCenter
                        width: parent.width
                        height: progressMouseArea.containsMouse || progressMouseArea.pressed ? 6 : 4
                        color: "#33FFFFFF"
                        radius: 3
                        Behavior on height { NumberAnimation { duration: 120 } }

                        Rectangle {
                            width: mediaPlayer.duration > 0 ?
                                   (progressMouseArea.dragPosition / mediaPlayer.duration) * parent.width : 0
                            height: parent.height
                            radius: parent.radius
                            gradient: Gradient {
                                orientation: Gradient.Horizontal
                                GradientStop { position: 0.0; color: colors.green }
                                GradientStop { position: 1.0; color: colors.superGreen }
                            }
                        }
                    }

                    Rectangle {
                        id: progressHandle
                        x: mediaPlayer.duration > 0 ?
                           (progressMouseArea.dragPosition / mediaPlayer.duration) * parent.width - width / 2 :
                           -width / 2
                        anchors.verticalCenter: parent.verticalCenter
                        width: progressMouseArea.containsMouse || progressMouseArea.pressed ? 16 : 0
                        height: width
                        radius: width / 2
                        color: colors.superGreen
                        Behavior on width { NumberAnimation { duration: 120 } }
                    }

                    MouseArea {
                        id: progressMouseArea
                        anchors.fill: parent
                        anchors.topMargin: -8
                        anchors.bottomMargin: -8
                        hoverEnabled: true
                        property real dragPosition: mediaPlayer.position

                        onPressed: (mouse) => {
                            dragPosition = (mouse.x / width) * mediaPlayer.duration
                            mediaPlayer.setPosition(Math.max(0, Math.min(dragPosition, mediaPlayer.duration)))
                        }
                        onPositionChanged: (mouse) => {
                            if (pressed) {
                                dragPosition = (mouse.x / width) * mediaPlayer.duration
                            }
                        }
                        onReleased: (mouse) => {
                            mediaPlayer.setPosition(Math.max(0, Math.min(dragPosition, mediaPlayer.duration)))
                        }
                    }
                }

                Text {
                    text: formatTime(mediaPlayer.duration)
                    color: "white"
                    font.pixelSize: 13
                    Layout.preferredWidth: 60
                }
            }

            // ──── Controls row ────
            Item {
                id: rowcontrols
                Layout.fillWidth: true
                Layout.preferredHeight: 56
                focus: true

                Component.onCompleted: rowcontrols.forceActiveFocus()

                Keys.onBackPressed: backButton.click()
                Keys.onEscapePressed: backButton.click()
                Keys.onLeftPressed: { if (leftButton.visible) leftButton.click() }
                Keys.onRightPressed: { if (nextButton.visible) nextButton.click() }
                Keys.onPressed: (event) => {
                    if (event.key === Qt.Key_MediaFastForward) ffButton.click()
                    if (event.key === Qt.Key_MediaRewind) rewindButton.click()
                }
                Keys.onDigit1Pressed: {
                    if (audioSelector.count > 0) {
                        audioSelector.currentIndex = (audioSelector.currentIndex + 1) % audioSelector.count
                        mediaPlayer.setAudioTrack(audioSelector.currentValue)
                    }
                }
                Keys.onDigit2Pressed: {
                    if (subtitleSelector.count > 0) {
                        subtitleSelector.currentIndex = (subtitleSelector.currentIndex + 1) % subtitleSelector.count
                        mediaPlayer.setSubtitleTrack(subtitleSelector.currentValue)
                    }
                }
                Keys.onDigit9Pressed: ffButton.click()
                Keys.onDigit7Pressed: rewindButton.click()
                Keys.onUpPressed: fullscreenButton.click()
                Keys.onDownPressed: mediaPlayer.setFullScreen(false)
                Keys.onVolumeUpPressed: volumeSlider.value = Math.min(100, volumeSlider.value + 10)
                Keys.onVolumeDownPressed: volumeSlider.value = Math.max(0, volumeSlider.value - 10)
                Keys.onReturnPressed: playpauseButton.clicked()

                // Centered playback group
                RowLayout {
                    anchors.centerIn: parent
                    spacing: 28

                    Button {
                        id: leftButton
                        visible: episodeType == "MiddleEpisode" || episodeType == "FinalEpisode"
                        onClicked: {
                            restartLoadingWindow()
                            root.lastEpisode()
                            if (root.mediaMetadata !== undefined) {
                                mediaPlayer.loadMedia(root.mediaId, root.mediaMetadata)
                            } else {
                                mediaPlayer.loadMedia(root.mediaId, {})
                            }
                        }
                        flat: true
                        Layout.preferredWidth: 26
                        Layout.preferredHeight: 26
                        contentItem: VectorImage {
                            source: parent.hovered ? "qrc:/media/buttons/lastone_hover.svg" : "qrc:/media/buttons/lastone.svg"
                            anchors.fill: parent
                            preferredRendererType: VectorImage.CurveRenderer
                        }
                        background: Rectangle { color: "transparent" }
                    }

                    Button {
                        id: rewindButton
                        onClicked: mediaPlayer.back30sec()
                        flat: true
                        Layout.preferredWidth: 30
                        Layout.preferredHeight: 30
                        contentItem: VectorImage {
                            source: parent.hovered ? "qrc:/media/buttons/back_hover.svg" : "qrc:/media/buttons/back.svg"
                            anchors.fill: parent
                            preferredRendererType: VectorImage.CurveRenderer
                        }
                        background: Rectangle { color: "transparent" }
                    }

                    Button {
                        id: playpauseButton
                        onClicked: mediaPlayer.isPlaying ? mediaPlayer.pauseMedia() : mediaPlayer.playMedia(0)
                        flat: true
                        Layout.preferredWidth: 52
                        Layout.preferredHeight: 52
                        // Note: original had play/pause icon names swapped, preserving that behavior.
                        contentItem: VectorImage {
                            source: parent.hovered
                                ? (mediaPlayer.isPlaying ? "qrc:/media/buttons/pause_hover.svg" : "qrc:/media/buttons/play_hover.svg")
                                : (mediaPlayer.isPlaying ? "qrc:/media/buttons/play.svg" : "qrc:/media/buttons/pause.svg")
                            anchors.fill: parent
                            preferredRendererType: VectorImage.CurveRenderer
                        }
                        background: Rectangle { color: "transparent" }
                    }

                    Button {
                        id: ffButton
                        onClicked: mediaPlayer.forward30sec()
                        flat: true
                        Layout.preferredWidth: 30
                        Layout.preferredHeight: 30
                        contentItem: VectorImage {
                            source: parent.hovered ? "qrc:/media/buttons/forward_hover.svg" : "qrc:/media/buttons/forward.svg"
                            anchors.fill: parent
                            preferredRendererType: VectorImage.CurveRenderer
                        }
                        background: Rectangle { color: "transparent" }
                    }

                    Button {
                        id: nextButton
                        visible: episodeType == "MiddleEpisode" || episodeType == "FirstEpisode"
                        onClicked: {
                            root.nextEpisode()
                            if (root.mediaMetadata !== undefined) {
                                mediaPlayer.loadMedia(root.mediaId, root.mediaMetadata)
                            } else {
                                mediaPlayer.loadMedia(root.mediaId, {})
                            }
                            restartLoadingWindow()
                        }
                        flat: true
                        Layout.preferredWidth: 26
                        Layout.preferredHeight: 26
                        contentItem: VectorImage {
                            source: parent.hovered ? "qrc:/media/buttons/next_hover.svg" : "qrc:/media/buttons/next.svg"
                            anchors.fill: parent
                            preferredRendererType: VectorImage.CurveRenderer
                        }
                        background: Rectangle { color: "transparent" }
                    }
                }

                // Right group: volume / settings / fullscreen
                RowLayout {
                    anchors.right: parent.right
                    anchors.verticalCenter: parent.verticalCenter
                    spacing: 14

                    VectorImage {
                        Layout.preferredWidth: 22
                        Layout.preferredHeight: 22
                        source: volumeSlider.value == 0 ? "qrc:/media/buttons/sound_0.svg" :
                                (volumeSlider.value < 51 ? "qrc:/media/buttons/sound_1.svg" : "qrc:/media/buttons/sound_2.svg")
                        preferredRendererType: VectorImage.CurveRenderer
                    }

                    Slider {
                        id: volumeSlider
                        Layout.preferredWidth: 110
                        Layout.preferredHeight: 22
                        from: 0
                        to: 100
                        value: 50
                        stepSize: 1
                        onValueChanged: mediaPlayer.setVolume(value)

                        background: Rectangle {
                            x: volumeSlider.leftPadding
                            y: volumeSlider.topPadding + volumeSlider.availableHeight / 2 - height / 2
                            width: volumeSlider.availableWidth
                            height: 3
                            radius: 2
                            color: "#33FFFFFF"

                            Rectangle {
                                width: volumeSlider.visualPosition * parent.width
                                height: parent.height
                                color: colors.superGreen
                                radius: 2
                            }
                        }

                        handle: Rectangle {
                            x: volumeSlider.leftPadding + volumeSlider.visualPosition * (volumeSlider.availableWidth - width)
                            y: volumeSlider.topPadding + volumeSlider.availableHeight / 2 - height / 2
                            implicitWidth: 14
                            implicitHeight: 14
                            radius: 7
                            color: "white"
                            border.color: colors.superGreen
                            border.width: volumeSlider.pressed ? 3 : 2
                            Behavior on border.width { NumberAnimation { duration: 120 } }
                        }
                    }

                    Button {
                        id: settingsButton
                        flat: true
                        Layout.preferredWidth: 26
                        Layout.preferredHeight: 26
                        onClicked: settingsPopup.opened ? settingsPopup.close() : settingsPopup.open()
                        contentItem: VectorImage {
                            source: "qrc:/media/buttons/audio.svg"
                            anchors.fill: parent
                            preferredRendererType: VectorImage.CurveRenderer
                        }
                        background: Rectangle { color: "transparent" }
                    }

                    Button {
                        id: fullscreenButton
                        onClicked: mediaPlayer.setFullScreen(!mediaPlayer.fullScreen)
                        flat: true
                        Layout.preferredWidth: 26
                        Layout.preferredHeight: 26
                        contentItem: VectorImage {
                            source: parent.hovered ? "qrc:/media/buttons/fullscreen_hover.svg" : "qrc:/media/buttons/fullscreen.svg"
                            anchors.fill: parent
                            preferredRendererType: VectorImage.CurveRenderer
                        }
                        background: Rectangle { color: "transparent" }
                    }
                }
            }
        }
    }

    // ─────────────── Settings popover (audio + subtitles) ───────────────
    Popup {
        id: settingsPopup
        parent: root
        x: root.width - width - 40
        y: root.height - 200 - height
        width: 320
        modal: false
        focus: true
        closePolicy: Popup.CloseOnPressOutsideParent | Popup.CloseOnEscape
        padding: 18

        background: Rectangle {
            color: "#F2050505"
            border.color: "#33FFFFFF"
            border.width: 1
            radius: 10
        }

        contentItem: ColumnLayout {
            spacing: 16

            ColumnLayout {
                Layout.fillWidth: true
                spacing: 6
                Text {
                    text: "Audio"
                    color: "white"
                    font.pixelSize: 13
                    font.bold: true
                }
                ComboBox {
                    id: audioSelector
                    Layout.fillWidth: true
                    Layout.preferredHeight: 32
                    model: mediaPlayer.audioTracks
                    textRole: "name"
                    valueRole: "id"
                    onActivated: mediaPlayer.setAudioTrack(currentValue)
                    background: Rectangle {
                        color: colors.surface
                        radius: 4
                        border.color: "#33FFFFFF"
                    }
                    contentItem: Text {
                        text: audioSelector.currentText
                        color: "white"
                        verticalAlignment: Text.AlignVCenter
                        leftPadding: 12
                        rightPadding: 24
                        elide: Text.ElideRight
                    }
                }
            }

            ColumnLayout {
                Layout.fillWidth: true
                spacing: 6
                Text {
                    text: "Subtitles"
                    color: "white"
                    font.pixelSize: 13
                    font.bold: true
                }
                ComboBox {
                    id: subtitleSelector
                    Layout.fillWidth: true
                    Layout.preferredHeight: 32
                    model: mediaPlayer.subtitleTracks
                    textRole: "name"
                    valueRole: "id"
                    currentIndex: 0
                    onActivated: mediaPlayer.setSubtitleTrack(currentValue)
                    background: Rectangle {
                        color: colors.surface
                        radius: 4
                        border.color: "#33FFFFFF"
                    }
                    contentItem: Text {
                        text: subtitleSelector.currentText
                        color: "white"
                        verticalAlignment: Text.AlignVCenter
                        leftPadding: 12
                        rightPadding: 24
                        elide: Text.ElideRight
                    }
                }
            }
        }
    }
}
