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
    property real loadingPos: -1
    property int loadingPosCounter: 0
    signal closeRequested
    signal mediaEnded
    signal nextEpisode
    signal lastEpisode
    signal updateMediaMetadata
    width: Screen.desktopAvailableWidth
    height: Screen.desktopAvailableHeight
    function loadingPosFun(){
        if (isLoading && loadingPos < mediaPlayer.position){
            loadingPos = mediaPlayer.position
            loadingPosCounter += 1
        }
        if (loadingPosCounter == 3){
            isLoading = false
        }
    }
    function restartLoadingWindow(){
        loadingPosCounter = 0
        loadingPos = -1
        isLoading = true
    }
    Window {
        id: loadingWindow
        visible: isLoading
        flags: Qt.FramelessWindowHint | Qt.Window
        color: "transparent" // Set transparent background for the window
        width: 1920
        height: 1080

        Timer {
            id: periodicTimer
            interval: 1000 // Call every 1000 ms (1 second)
            running: true
            repeat: true
            onTriggered: {
                console.log("TIMER")
                root.loadingPosFun()
            }
        }


        Rectangle {
            id: loadingScreen
            color: "black"
            anchors.fill: parent

            Image {
                id: loadingImage
                source: "qrc:/media/loading.png" // Path to your loading image
                anchors.centerIn: parent
                scale: 0.5
                opacity: 0 // Initially transparent

                transform: Rotation {
                    id: rotation
                    origin.x: loadingImage.width / 2
                    origin.y: loadingImage.height / 2
                    angle: 0
                }
            }

            ParallelAnimation  {
                id: fadeInAndRotate
                loops: Animation.Infinite // Infinite rotation after fade-in

                // Step 1: Fade-in effect
                NumberAnimation {
                    target: loadingImage
                    property: "opacity"
                    from: 0
                    to: 1
                    duration: 1000 // 1 second fade-in
                }

                // Step 2: Start rotation
                RotationAnimation {
                    target: rotation
                    property: "angle"
                    from: 0
                    to: 360
                    duration: 3000 // 3 seconds per rotation
                    loops: Animation.Infinite
                }
            }

            Timer {
                id: startDelayTimer
                interval: 100 // 3 seconds delay
                running: true
                repeat: false
                onTriggered: {
                    fadeInAndRotate.start(); // Start the fade-in and rotation animation after the timer ends
                }
            }
        }



        // Ensure the window stays in sync with the parent
        Component.onCompleted: {
            let globalPos = root.mapToGlobal(Qt.point(0, 0));
            x = globalPos.x;
            y = globalPos.y;
            root.widthChanged.connect(() => width = root.width);
            root.heightChanged.connect(() => height = root.height);
            //isLoading = false
            
        }
    }

    // Create a basic column layout
    ColumnLayout {
        anchors.fill: parent
        spacing: 0
        width: Screen.desktopAvailableWidth
        height: Screen.desktopAvailableHeight
        // Video area
        Rectangle {
            Layout.fillWidth: true
            // Use Layout.preferredHeight to make it take remaining space minus controls
            Layout.preferredHeight: parent.height - 160
            color: "black"

            VideoOutput {
                id: videoOutput
                anchors.fill: parent
                visible: false
            }

            VLCPlayerHandler {
                id: mediaPlayer
                
               
                
                Component.onCompleted: {
                    
                    if (root.mediaId) {
                        if (root.mediaMetadata !== undefined) {
                            loadMedia(root.mediaId, root.mediaMetadata)
                        } else {
                            loadMedia(root.mediaId, {})
                        }
                        videoSink = videoOutput.videoSink
                        //audioSelector.currentIndex = mediaPlayer.getAudioIndex()
                        //audioSelector.currentText = mediaPlayer.getAudioText()
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
            
        }

        // Controls area with fixed height
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 160
            color: "#CC000000"
            Layout.preferredWidth: Screen.desktopAvailableWidth
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
                            color: "#1E1E1E"
                            radius: 2

                            Rectangle {
                                width: mediaPlayer.duration > 0 ?
                                       (progressMouseArea.dragPosition / mediaPlayer.duration) * parent.width : 0
                                height: parent.height
                                gradient: Gradient {
                                    orientation: Gradient.Horizontal
                                    GradientStop { position: 1.0; color: colors.superGreen }
                                    GradientStop { position: 0.8; color: colors.green }
                                    GradientStop { position: 0.0; color: colors.green }
                                }
                                radius: 2
                            }
                        }

                        Rectangle {
                            id: handle
                            x: mediaPlayer.duration > 0 ?
                               (progressMouseArea.dragPosition / mediaPlayer.duration) * (parent.width - width) : 0
                            anchors.verticalCenter: parent.verticalCenter
                            width: progressMouseArea.pressed ? 20 : 16
                            height: width
                            radius: width / 2
                            color: colors.superGreen
                        }

                        MouseArea {
                            id: progressMouseArea
                            anchors.fill: parent
                            hoverEnabled: true
                            property bool moved: false
                            property real dragPosition: mediaPlayer.position // Temporary position during drag

                            onPressed: {
                                // Calculate position immediately on press
                                dragPosition = (mouseX / width) * mediaPlayer.duration
                                mediaPlayer.setPosition(Math.max(0, Math.min(dragPosition, mediaPlayer.duration)))
                            }

                            onPositionChanged: {
                                if (pressed) {
                                    // Update drag position dynamically
                                    dragPosition = (mouseX / width) * mediaPlayer.duration
                                }
                            }

                            onReleased: {
                                // Commit the new position when released
                                mediaPlayer.setPosition(Math.max(0, Math.min(dragPosition, mediaPlayer.duration)))
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
                    //spacing: 16
                    Layout.alignment: Qt.AlignVCenter
                    Button {
                        //text: "←"
                        onClicked: {
                            mediaPlayer.updateMediaMetadata()
                            root.closeRequested()
                        }
                        flat: true
                        Layout.preferredWidth: 24
                        Layout.preferredHeight: 24
                        contentItem: VectorImage {
                            //height: 6
                            //width: 6
                            source: parent.hovered ? "qrc:/media/buttons/left_hover.svg" : "qrc:/media/buttons/left.svg"
                            anchors.fill: parent // Make the VectorImage fill the Button
                            preferredRendererType: VectorImage.CurveRenderer
                            
                        }
                        background: Rectangle {
                            color: "transparent"
                        }
                    }
                    Item { Layout.preferredWidth: 800 }
                    Button {
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
                        Layout.preferredWidth: 16
                        Layout.preferredHeight: 16
                        contentItem: VectorImage {
                            //height: 6
                            //width: 6
                            source: parent.hovered ? "qrc:/media/buttons/lastone_hover.svg" : "qrc:/media/buttons/lastone.svg"
                            anchors.fill: parent // Make the VectorImage fill the Button
                            preferredRendererType: VectorImage.CurveRenderer
                            
                        }
                        background: Rectangle {
                            color: "transparent"
                        }
                    }
                    Button {
                        onClicked: mediaPlayer.back30sec();
                        flat: true
                        Layout.preferredWidth: 24
                        Layout.preferredHeight: 24
                        contentItem: VectorImage {
                            //height: 6
                            //width: 6
                            source: parent.hovered ? "qrc:/media/buttons/back_hover.svg" : "qrc:/media/buttons/back.svg"
                            anchors.fill: parent // Make the VectorImage fill the Button
                            preferredRendererType: VectorImage.CurveRenderer
                            
                        }
                        background: Rectangle {
                            color: "transparent"
                        }
                    }
                    Button {
                        onClicked: mediaPlayer.isPlaying ? mediaPlayer.pauseMedia() : mediaPlayer.playMedia(0)
                        flat: true
                        Layout.preferredWidth: 24
                        Layout.preferredHeight: 24
                        padding: 0
                        background: Rectangle {
                            color: "transparent" // Transparent background
                            
                        }
                        
                        contentItem: VectorImage {
                            //height: 6
                            //width: 6
                            source: parent.hovered
                                ? (mediaPlayer.isPlaying ? "qrc:/media/buttons/pause_hover.svg" : "qrc:/media/buttons/play_hover.svg")
                                : (mediaPlayer.isPlaying ? "qrc:/media/buttons/play.svg" : "qrc:/media/buttons/pause.svg")
                            anchors.fill: parent // Make the VectorImage fill the Button
                            preferredRendererType: VectorImage.CurveRenderer
                            
                        }
                    }
                    Button {
                        onClicked: mediaPlayer.forward30sec();
                        flat: true
                        Layout.preferredWidth: 24
                        Layout.preferredHeight: 24
                        contentItem: VectorImage {
                            //height: 6
                            //width: 6
                            source: parent.hovered ? "qrc:/media/buttons/forward_hover.svg" : "qrc:/media/buttons/forward.svg"
                            anchors.fill: parent // Make the VectorImage fill the Button
                            preferredRendererType: VectorImage.CurveRenderer
                            
                        }
                        background: Rectangle {
                            color: "transparent"
                        }
                    }
                    
                    Button {
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
                        Layout.preferredWidth: 16
                        Layout.preferredHeight: 16
                        contentItem: VectorImage {
                            //height: 6
                            //width: 6
                            source: parent.hovered ? "qrc:/media/buttons/next_hover.svg" : "qrc:/media/buttons/next.svg"
                            anchors.fill: parent // Make the VectorImage fill the Button
                            preferredRendererType: VectorImage.CurveRenderer
                            
                        }
                        background: Rectangle {
                            color: "transparent"
                        }
                    }
                    Item { Layout.preferredWidth: 24 }
                    Button {
                        onClicked: mediaPlayer.setFullScreen(true)
                        flat: true
                        Layout.preferredWidth: 24
                        Layout.preferredHeight: 24
                        contentItem: VectorImage {
                            //height: 6
                            //width: 6
                            source: parent.hovered ? "qrc:/media/buttons/fullscreen_hover.svg" : "qrc:/media/buttons/fullscreen.svg"
                            anchors.fill: parent // Make the VectorImage fill the Button
                            preferredRendererType: VectorImage.CurveRenderer
                            
                        }
                        background: Rectangle {
                            color: "transparent"
                        }
                    }
                    Item { Layout.fillWidth: true }
                    RowLayout {
                        Layout.fillWidth: true
                        Layout.alignment: Qt.AlignRight
                        Button {
                            //onClicked: 
                            flat: true
                            Layout.preferredWidth: 24
                            Layout.preferredHeight: 24
                            contentItem: VectorImage {
                                //height: 6
                                //width: 6
                                source: volumeSlider.value == 0 ? "qrc:/media/buttons/sound_0.svg" : (volumeSlider.value < 51 ? "qrc:/media/buttons/sound_1.svg" : "qrc:/media/buttons/sound_2.svg")                          
                                anchors.fill: parent // Make the VectorImage fill the Button
                                preferredRendererType: VectorImage.CurveRenderer
                            
                            }
                            background: Rectangle {
                                color: "transparent"
                            }
                        }
                        Rectangle {
                            //Layout.fillWidth: true
                            Layout.preferredHeight: 24
                            Layout.preferredWidth: 150
                            color: "transparent"
                            Slider {
                                id: volumeSlider
                                anchors.centerIn: parent
                                width: 150
                                from: 0
                                to: 100
                                value: 50
                                stepSize: 1
                                onValueChanged: {
                                    console.log("Volume: " + value)
                                    mediaPlayer.setVolume(value)
                                }

                                

                            

        
                                handle: Rectangle {
                                    x: volumeSlider.leftPadding + (volumeSlider.horizontal ? volumeSlider.visualPosition * (volumeSlider.availableWidth - width) : (volumeSlider.availableWidth - width) / 2)
                                    y: volumeSlider.topPadding + (volumeSlider.vertical ? volumeSlider.visualPosition * (volumeSlider.availableHeight - height) : (volumeSlider.availableHeight - height) / 2)

                                    implicitWidth: 15
                                    implicitHeight: 15

                                    radius: width/2

                                    border.width: volumeSlider.pressed ? width/2 : 1
                                    border.color: volumeSlider.background.color
                                    color: colors.surface
                                    Behavior on border.width { SmoothedAnimation {} }

                                    
                                }

                                background: Rectangle {
                                    id: bgcolor
                                    x: (volumeSlider.width  - width) / 2
                                    y: (volumeSlider.height - height) / 2

                                    implicitWidth: volumeSlider.horizontal ? 200 : 1
                                    implicitHeight: volumeSlider.horizontal ? 1 : 200

                                    width: volumeSlider.horizontal ? volumeSlider.availableWidth : implicitWidth
                                    height: volumeSlider.horizontal ? implicitHeight : volumeSlider.availableHeight

                                    radius: width

                                
                                }
                            }
                        }
                    }
                    
                    Item { Layout.fillWidth: true }
                    Button {
                        //onClicked: 
                        flat: true
                        Layout.preferredWidth: 24
                        Layout.preferredHeight: 24
                        contentItem: VectorImage {
                            //height: 6
                            //width: 6
                            source: "qrc:/media/buttons/audio.svg"                            
                            anchors.fill: parent // Make the VectorImage fill the Button
                            preferredRendererType: VectorImage.CurveRenderer
                            
                        }
                        background: Rectangle {
                            color: "transparent"
                        }
                    }
                    
                    ComboBox {
                        id: audioSelector
                        Layout.preferredWidth: 150
                        Layout.preferredHeight: 24
                        model: mediaPlayer.audioTracks
                        textRole: "name"
                        valueRole: "id"
                        //currentIndex: mediaPlayer.getAudioIndex()
                        onActivated: mediaPlayer.setAudioTrack(currentValue)
                        background: Rectangle {
                            z: -1
                            color: colors.background
                            radius: 2
                            border.color: "white"
                            //border.width: 4
                        }
                        contentItem: Text {
                            text: audioSelector.currentText
                            color: "white"         // Text color
                            verticalAlignment: Text.AlignVCenter
                            horizontalAlignment: Text.AlignLeft
                            elide: Text.ElideRight
                            anchors.leftMargin: 8
                            anchors.left: parent.left
                            anchors.verticalCenter: parent.verticalCenter
                        }
                        
                    }
                    Button {
                        //onClicked: 
                        flat: true
                        Layout.preferredWidth: 24
                        Layout.preferredHeight: 24
                        contentItem: VectorImage {
                            //height: 6
                            //width: 6
                            source: "qrc:/media/buttons/subtitles.svg"                            
                            anchors.fill: parent // Make the VectorImage fill the Button
                            preferredRendererType: VectorImage.CurveRenderer
                            
                        }
                        background: Rectangle {
                            color: "transparent"
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
                            mediaPlayer.setSubtitleTrack(currentValue)        
                        }
                        Layout.preferredHeight: 24
                        background: Rectangle {
                            z: -1
                            color: colors.background
                            radius: 2
                            border.color: "white"
                            //border.width: 4
                        }
                        contentItem: Text {
                            text: currentValue
                            color: "white"         // Text color
                            verticalAlignment: Text.AlignVCenter
                            horizontalAlignment: Text.AlignLeft
                            elide: Text.ElideRight
                            anchors.leftMargin: 8
                            anchors.left: parent.left
                            anchors.verticalCenter: parent.verticalCenter
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