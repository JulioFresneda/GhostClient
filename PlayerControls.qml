import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: controlsRoot
    color: "transparent"
    
    required property bool isPlaying
    required property real position
    required property real duration
    required property string title
    
    signal playPauseClicked()
    signal positionRequested(real position)
    signal closeRequested()
    
    // Top controls
    Rectangle {
        id: topControls
        height: 60
        width: parent.width
        color: "#80000000"
        opacity: 1 //controlsVisible ? 1 : 0
        
        Behavior on opacity {
            NumberAnimation { duration: 200 }
        }
        
        RowLayout {
            anchors.fill: parent
            anchors.margins: 10
            spacing: 8
            
            Button {
                text: "← Back"
                onClicked: controlsRoot.closeRequested()
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
                text: controlsRoot.title
                color: "white"
                font.pointSize: 14
            }
        }
    }
    
    // Bottom controls
    Rectangle {
        id: bottomControls
        anchors.bottom: parent.bottom
        width: parent.width
        height: 80
        color: "#80000000"
        opacity: 1 //controlsVisible ? 1 : 0
        
        Behavior on opacity {
            NumberAnimation { duration: 200 }
        }
        
        RowLayout {
            anchors.fill: parent
            anchors.margins: 20
            spacing: 20
            
            Button {
                text: controlsRoot.isPlaying ? "⏸" : "▶"
                onClicked: controlsRoot.playPauseClicked()
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
                text: formatTime(controlsRoot.position) + " / " + formatTime(controlsRoot.duration)
                color: "white"
                font.pointSize: 12
            }
            
            Slider {
                Layout.fillWidth: true
                from: 0
                to: controlsRoot.duration
                value: controlsRoot.position
                onMoved: controlsRoot.positionRequested(value)
                
                background: Rectangle {
                    x: parent.leftPadding
                    y: parent.topPadding + parent.availableHeight / 2 - height / 2
                    width: parent.availableWidth
                    height: 4
                    radius: 2
                    color: "#404040"
                    
                    Rectangle {
                        width: parent.width * (controlsRoot.position / controlsRoot.duration)
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
    
    // Controls visibility state
    property bool controlsVisible: true
    Timer {
        id: controlsTimer
        interval: 3000
        onTriggered: controlsVisible = false
    }
    
    // Mouse area for showing controls
    MouseArea {
        anchors.fill: parent
        hoverEnabled: true
        onPositionChanged: {
            controlsTimer.restart()
            controlsVisible = true
        }
    }
    
    function formatTime(ms) {
        let seconds = Math.floor(ms / 1000)
        let minutes = Math.floor(seconds / 60)
        seconds = seconds % 60
        return minutes.toString().padStart(2, '0') + ':' + 
               seconds.toString().padStart(2, '0')
    }
}