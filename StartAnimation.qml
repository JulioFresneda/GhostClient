// SplashScreen.qml
import QtQuick 2.15
import QtQuick.Controls 2.15

Rectangle {
    id: splash
    color: "#000000"
    signal animationFinished

    Item {
        id: container
        anchors.centerIn: parent
        width: logo.width + ghostText.width + streamText.width + 40
        height: Math.max(logo.height, ghostText.height, streamText.height)

        Image {
            id: logo
            source: "qrc:/media/logo.png"
            width: 300
            height: 500
            anchors.centerIn: parent
            opacity: 0
            fillMode: Image.PreserveAspectFit

            RotationAnimation on rotation {
                id: rotationAnim
                from: 0
                to: 360
                duration: 2000
                loops: 1
                running: false
            }

            NumberAnimation on opacity {
                id: logoFadeIn
                from: 0
                to: 1
                duration: 4000
                easing.type: Easing.OutQuad
                onFinished: {
                    rotationAnim.start()
                    ghostFadeIn.start()
                    streamFadeIn.start()
                }
            }
        }

        Text {
            id: ghostText
            text: "Ghost"
            color: "#FFFFFF"
            font { pointSize: 72; weight: Font.Light }
            anchors {
                right: logo.left
                rightMargin: 20
                verticalCenter: parent.verticalCenter
            }
            opacity: 0
            x: logo.x + logo.width/2  // Start position at center

       

            NumberAnimation {
                id: ghostFadeIn
                target: ghostText
                property: "opacity"
                from: 0
                to: 1
                duration: 4000
                easing.type: Easing.OutQuad
            }
            
        }

        Text {
            id: streamText
            text: "Stream"
            color: "#FFFFFF"
            font { pointSize: 72; weight: Font.Light }
            anchors {
                left: logo.right
                leftMargin: 20
                verticalCenter: parent.verticalCenter
            }
            opacity: 0
            x: logo.x + logo.width/2  // Start position at center

            NumberAnimation {
                id: streamFadeIn
                target: streamText
                property: "opacity"
                from: 0
                to: 1
                duration: 4000
                easing.type: Easing.OutQuad
                onFinished: {
                    rotationAnim.stop()
                    finalTimer.start()
                }
                
            }
        }
    }

    Timer {
        id: finalTimer
        interval: 1000
        repeat: false
        onTriggered: splash.animationFinished()
    }
}