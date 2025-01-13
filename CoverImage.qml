import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Controls.Basic

Item {
    id: root
    property string mediaId: ""
    property string backupId: ""
    property string coverImageBase64: ""
    property bool isLoading: true
    property bool hasError: false

    Image {
        id: coverImage
        anchors.fill: parent
        fillMode: Image.PreserveAspectCrop
        cache: true
        asynchronous: true
        visible: !isLoading && !hasError
        source: coverImageBase64

        onStatusChanged: {
            if (status === Image.Ready) {
                root.isLoading = false
            } else if (status === Image.Error) {
                root.hasError = true
                root.isLoading = false
            }
        }
    }

    Rectangle {
        anchors.fill: parent
        color: "#2A2A2A"
        visible: isLoading

        BusyIndicator {
            anchors.centerIn: parent
            running: isLoading
            width: 32
            height: 32
        }
    }

    Rectangle {
        anchors.fill: parent
        color: "#2A2A2A"
        visible: hasError

        Text {
            anchors.centerIn: parent
            text: "🖼️"
            color: "#666666"
            font.pixelSize: 32
        }
    }

    Connections {
        target: loginManager
        function onCoverImageLoaded(loadedMediaId, base64Data) {
            if (loadedMediaId === root.mediaId) {
                if(coverImage){
                    coverImage.source = "data:image/jpeg;base64," + base64Data
                }
            }
        }
        
        function onCoverImageError(loadedMediaId) {
            if (loadedMediaId === root.mediaId) {
                root.hasError = true
                root.isLoading = false
            }
        }
    }

    Component.onCompleted: {
        if (mediaId) {
            if (coverImageBase64 == ""){
                //loginManager.loadCoverImage(mediaId, backupId)
            }
        }
    }
}