import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Controls.Basic
import QtMultimedia
import com.ghoststream 1.0
import "." 

Item {
    id: root
    width: 1920
    height: 1080

    // Properties for data management
    property var collectionsData: []
    property var mediaData: []
    property var mediaMetadata: []
    property var filteredData: []
    property string currentCategory: "continueWatching"
    property bool isGridView: true
    property string selectedCollectionId: ""
    property string selectedCollectionTitle: ""
    property bool isPlayerVisible: false
    property string currentMediaId: ""

    property var categories: {
        "continueWatching": "Continuar viendo",
        "series" : "Series",
        "movies" : "Pelis"
    }

    // Color scheme
    QtObject {
        id: colors
        property string background: "#050505"
        property string surface: "#1E1E1E"
        property string primary: "#FCF7F8"
        property string textPrimary: "#050505"
        property string textSecondary: "#FCF7F8"
        property string strongViolet: "#290D3D"
        property string strongWhite: "#e2e2e2"
        property string green: "#419A38"
    }

    // Background
    Rectangle {
        anchors.fill: parent
        color: colors.background
    }

    // Main Layout
    RowLayout {
        id: mainContent
        anchors.fill: parent
        visible: !isPlayerVisible
        spacing: 0

        // Sidebar
        Rectangle {
            Layout.preferredWidth: 240
            Layout.fillHeight: true
            color: colors.surface

            Image {
                anchors.fill: parent
                id: sidebar
                source: "qrc:/media/sidebar.png"
            }

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 16
                spacing: 24

                Item {
                    Layout.preferredHeight: parent.height * 0.2 // Takes 3/4 of the height
                }

                // Categories
                Repeater {
                    model: Object.keys(categories)
                    delegate: ItemDelegate {
                        Layout.fillWidth: true
                        height: 48

                        background: Rectangle {
                            color: currentCategory === modelData ? 
                                  colors.primary : colors.background
                            radius: 8
                        }

                        contentItem: Text {
                            text: categories[modelData]
                            color: currentCategory === modelData ? 
                                  colors.textPrimary : colors.textSecondary
                            font.pointSize: 14
                            verticalAlignment: Text.AlignVCenter
                        }

                        onClicked: {
                            currentCategory = modelData
                            filterContent()
                        }
                    }
                }

                Item { Layout.fillHeight: true } // Spacer

                ItemDelegate {
                    Layout.fillWidth: true
                    height: 48
                    visible: selectedCollectionId !== ""

                    background: Rectangle {
                        color: "transparent"
                        radius: 8
                    }

                    contentItem: RowLayout {
                        spacing: 8
                        Text {
                            text: "←"
                            color: colors.textPrimary
                            font.pointSize: 16
                        }
                        Text {
                            text: "Back"
                            color: colors.textPrimary
                            font.pointSize: 14
                        }
                    }

                    onClicked: {
                        selectedCollectionId = ""
                        selectedCollectionTitle = ""
                        filterContent()
                    }
                }
            }
        }

        // Main Content
        ColumnLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.margins: 24
            spacing: 24

            

            // Search and View Toggle Bar
            Rectangle {
                Layout.fillWidth: true
                height: 56
                color: colors.surface
                radius: 8

                RowLayout {
                    anchors.fill: parent
                    anchors.margins: 8
                    spacing: 16

                    Text {
                        Layout.fillWidth: true
                        text: selectedCollectionTitle
                        color: colors.textPrimary
                        font {
                            pointSize: 24
                            bold: true
                        }
                        visible: selectedCollectionId !== ""
                    }

                    TextField {
                        id: searchField
                        Layout.fillWidth: true
                        placeholderText: "Search content..."
                        placeholderTextColor: colors.textSecondary
                        color: colors.textPrimary
                        font.pointSize: 12

                        background: Rectangle {
                            color: colors.background
                            radius: 4
                        }

                        onTextChanged: filterContent()
                    }

                    CheckBox {
                        id: groupMoviesCheck
                        text: "Group by Collection"
                        visible: currentCategory === "movies"
                        checked: true

                        contentItem: Text {
                            text: groupMoviesCheck.text
                            color: colors.textPrimary
                            leftPadding: groupMoviesCheck.indicator.width + 4
                            verticalAlignment: Text.AlignVCenter
                        }

                        indicator: Rectangle {
                            implicitWidth: 20
                            implicitHeight: 20
                            radius: 3
                            border.color: colors.primary
                            color: "transparent"

                            Rectangle {
                                width: 12
                                height: 12
                                anchors.centerIn: parent
                                radius: 2
                                color: colors.primary
                                visible: groupMoviesCheck.checked
                            }
                        }

                        onCheckedChanged: filterContent()
                    }

                    Row {
                        spacing: 8

                        Button {
                            width: 40
                            height: 40
                            text: "⊞"
                            flat: true
                            checked: isGridView
                            onClicked: isGridView = true
                        }

                        Button {
                            width: 40
                            height: 40
                            text: "≣"
                            flat: true
                            checked: !isGridView
                            onClicked: isGridView = false
                        }
                    }
                }
            }
// Content Grid/List View
            Rectangle {
                Layout.fillWidth: true
                Layout.fillHeight: true
                color: colors.surface
                radius: 8
                clip: true

                GridView {
                    id: mediaGrid
                    anchors.fill: parent
                    cellWidth: 200
                    cellHeight: 300
                    model: filteredData.length > 0 ? filteredData : mediaData

                    delegate: MediaCard {
                        width: 180
                        height: 280
                        title: selectedCollectionId ? 
                               (modelData.title || "") :
                               (currentCategory === "series" || 
                               (currentCategory === "movies" && groupMoviesCheck.checked && modelData.collection_title)) ?
                               modelData.collection_title || "" :
                               modelData.title || ""
                        mediaId: modelData.ID || ""
                        property bool isCollection: modelData.collection_title ? true : false
                    }
                }

                ListView {
                    anchors.fill: parent
                    anchors.margins: 16
                    visible: !isGridView
                    model: filteredData
                    clip: true

                    delegate: MediaListItem {
                        width: parent.width - 32
                        height: 80
                        title: modelData.title || "Untitled"
                        description: modelData.description || ""
                        mediaId: modelData.ID
                        property string collectionName: {
                            if (modelData.collection_id) {
                                let collection = root.collectionsData.find(c => c.ID === modelData.collection_id)
                                return collection ? collection.collection_title : ""
                            }
                            return ""
                        }
                        onClicked: showMediaDetails(modelData)
                    }

                    ScrollBar.vertical: ScrollBar {}
                }
            }
        }
    }


    Rectangle {
        id: playerContainer
        anchors.fill: parent
        visible: isPlayerVisible
        z: isPlayerVisible ? 100 : -1

        Loader {
            id: playerLoader
            anchors.fill: parent
            active: isPlayerVisible
            sourceComponent: Component {
                MediaPlayer {
                    mediaId: currentMediaId
                    title: selectedCollectionTitle
                    mediaMetadata: root.mediaMetadata[currentMediaId]
                    onCloseRequested: {
                        isPlayerVisible = false
                        currentMediaId = ""
                    }
                    onMediaEnded: {
                        if (selectedCollectionId) {
                            // Find current media in filtered data
                            let currentIndex = filteredData.findIndex(media => media.ID === currentMediaId)
                            if (currentIndex >= 0 && currentIndex < filteredData.length - 1) {
                                // Play next episode
                                currentMediaId = filteredData[currentIndex + 1].ID
                                mediaId = currentMediaId
                                mediaMetadata = root.mediaMetadata[currentMediaId]
                            }
                        }
                    }
                    
                }
            }
        }

        
    }

    // Components for grid and list items
    component MediaCard: Rectangle {
        id: card
        property string title: ""
        property string mediaId: ""
        signal clicked
        color: "white"
        border.width: 5
        bottomLeftRadius: 50
        bottomRightRadius: 50
        border.color: cardMouseArea.containsMouse ? colors.green : colors.strongWhite

        gradient: Gradient {
            //GradientStop { position: 0.0; color: colors.strongWhite }
            GradientStop { position: 0.84; color: colors.strongWhite }
            GradientStop { position: 0.88; color: "white" }
        }

        // Main content container with border
        Rectangle {
            id: contentContainer
            anchors {
                fill: parent
                margins: 5  // Add some margin to ensure border is visible
            }
            color: "transparent"
        
            // Cover image container
            Rectangle {
                id: imageContainer
                anchors {
                    top: contentContainer.top
                    left: contentContainer.left
                    right: contentContainer.right
                }
                height: parent.height - titleContainer.height

                color: colors.background
                clip: true  // Ensure image stays within bounds

                CoverImage {
                    anchors.fill: parent
                    mediaId: card.mediaId
                }
            }

            // Title container with white background
            Rectangle {
                id: titleContainer
                anchors {
                    left: contentContainer.left
                    right: contentContainer.right
                    bottom: contentContainer.bottom
                }
                height: titleText.height + 16
                color: "transparent"
                clip: true
                Rectangle {
                    id: progressBar
                    anchors {
                        left: parent.left
                        right: parent.right
                        bottom: parent.top
                    }
                    height: 4
                    color: "#333333"
                    visible: getMediaProgress(card.mediaId) > 0
    
                    Rectangle {
                        id: progressFill
                        anchors {
                            left: parent.left
                            top: parent.top
                            bottom: parent.bottom
                        }
                        width: parent.width * getMediaProgress(card.mediaId)
                        color: "#1DB954"
                    }
                }

                Text {
                    id: titleText
                    anchors {
                        left: parent.left
                        right: parent.right
                        verticalCenter: parent.verticalCenter
                        margins: 6
                    }
                    text: title
                    color: "black"
                    font.pointSize: 12
                    horizontalAlignment: Text.AlignHCenter
                    elide: Text.ElideRight

                    wrapMode: Text.WordWrap
                    
                }

                
                
            }
        }

        MouseArea {
            id: cardMouseArea
            anchors.fill: parent
            hoverEnabled: true
            onClicked: {
                console.log("MediaCard clicked:", mediaId, "Collection:", modelData.collection_title)
                if (selectedCollectionId) {
                    console.log("Loading media from collection:", modelData.ID)
                    currentMediaId = modelData.ID
                    isPlayerVisible = true
                } else if (modelData.collection_title) {
                    selectedCollectionId = modelData.ID
                    selectedCollectionTitle = modelData.collection_title
                    filterContent()
                } else {
                    console.log("Loading standalone media:", modelData.ID)
                    currentMediaId = modelData.ID
                    isPlayerVisible = true
                }
            }
        }
    }

    // Component for list items
    component MediaListItem: Rectangle {
        id: listItem
        property string title: ""
        property string description: ""
        property string mediaId: ""
        property string collectionName: ""
        signal clicked

        color: colors.surface
        radius: 4

        RowLayout {
            anchors.fill: parent
            anchors.margins: 8
            spacing: 16

            Rectangle {
                width: 100
                height: 56
                color: colors.background
                radius: 4

                CoverImage {
                    anchors.fill: parent
                    mediaId: listItem.mediaId
                }
            }

            ColumnLayout {
                Layout.fillWidth: true
                spacing: 4

                Text {
                    Layout.fillWidth: true
                    text: title
                    color: colors.textPrimary
                    font.pointSize: 12
                    elide: Text.ElideRight
                }

                Text {
                    Layout.fillWidth: true
                    text: collectionName
                    color: colors.textSecondary
                    font.pointSize: 10
                    visible: collectionName !== ""
                    elide: Text.ElideRight
                }

                Text {
                    Layout.fillWidth: true
                    text: description
                    color: colors.textSecondary
                    font.pointSize: 9
                    elide: Text.ElideRight
                    maximumLineCount: 2
                    wrapMode: Text.WordWrap
                }
            }
        }

        MouseArea {
            anchors.fill: parent
            hoverEnabled: true
            onClicked: listItem.clicked()
            onEntered: listItem.state = "hover"
            onExited: listItem.state = ""
        }

        states: State {
            name: "hover"
            PropertyChanges {
                target: listItem
                color: Qt.darker(colors.surface, 1.2)
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

    function filterContent() {
        let searchText = searchField.text.toLowerCase()

        if (selectedCollectionId !== "") {
            let selectedCollection = collectionsData.find(c => c.ID === selectedCollectionId)
            if (selectedCollection) {
                filteredData = mediaData.filter(media => {
                    let matchesCollection = media.collection_id === selectedCollectionId
                    let matchesSearch = media.title.toLowerCase().includes(searchText)
                    return matchesCollection && matchesSearch
                })

                filteredData.sort((a, b) => {
                    if (a.season !== b.season) {
                        return (a.season || 0) - (b.season || 0)
                    }
                    return (a.episode || 0) - (b.episode || 0)
                })
            }
            return
        }
    
        console.log("Filtering with:", {
            mediaDataLength: mediaData.length,
            collectionsDataLength: collectionsData.length,
            searchText: searchText,
            currentCategory: currentCategory,
            groupMovies: groupMoviesCheck.checked
        })
        if (currentCategory === "continueWatching") {
            // Filter media items that are in progress
            filteredData = mediaData.filter(media => {
                let matchesSearch = (media.title || "").toLowerCase().includes(searchText);
                let progress = getMediaProgress(media.ID);
        
                // Show items with progress between 0% and 99%
                return matchesSearch && progress > 0 && progress < 0.99;
            });
    
            // Sort by progress (most recently watched first)
            filteredData.sort((a, b) => {
                // Get progress for both items
                let progressA = getMediaProgress(a.ID);
                let progressB = getMediaProgress(b.ID);
        
                // Sort in descending order (higher progress first)
                return progressB - progressA;
            });
        }
        else if (currentCategory === "series") {
            filteredData = collectionsData.filter(collection => {
                let matchesSearch = collection.collection_title.toLowerCase().includes(searchText)
                let isSeries = collection.collection_type === "serie"
                return matchesSearch && isSeries
            })
        } 
        else if (currentCategory === "movies" && groupMoviesCheck.checked) {
            let movieCollections = collectionsData.filter(collection => {
                let matchesSearch = collection.collection_title.toLowerCase().includes(searchText)
                let isMovies = collection.collection_type === "movies"
                return matchesSearch && isMovies
            })

            let standaloneMovies = mediaData.filter(media => {
                let matchesSearch = (media.title || "").toLowerCase().includes(searchText)
                return !media.collection_id && media.type === "movie" && matchesSearch
            })

            filteredData = [...movieCollections, ...standaloneMovies]
        }
        else {
            filteredData = mediaData.filter(media => {
                let title = media.title || ""
                let matchesSearch = title.toLowerCase().includes(searchText)

                let matchesCategory = true
                if (currentCategory === "movies") {
                    matchesCategory = media.type === "movie"
                }

                return matchesSearch && matchesCategory
            })
        }

        console.log("Filtered results:", filteredData.length, "items")
    }

    function showMediaDetails(mediaItem) {
        console.log("Selected media:", JSON.stringify(mediaItem, null, 2))
        currentMediaId = mediaItem.ID
        isPlayerVisible = true
    }

    Connections {
        target: loginManager

        function onMediaMetadataFetched(metadata) {
            let metadataMap = {};
            const metadataArray = metadata || [];
        
            for (let i = 0; i < metadataArray.length; i++) {
                const meta = metadataArray[i];
                metadataMap[meta.mediaID] = meta;
            }
        
            root.mediaMetadata = metadataMap;
            console.log("Loaded media metadata:", Object.keys(root.mediaMetadata).length);
            // Initial filtering
            filterContent()
        }

        function onMediaDataFetched(fetchedData) {
            console.log("Received media data:", JSON.stringify(fetchedData, null, 2))

            if (fetchedData.collections) {
                root.collectionsData = fetchedData.collections
                console.log("Loaded collections:", root.collectionsData.length)
            }

            if (fetchedData.media) {
                root.mediaData = fetchedData.media
                console.log("Total media items loaded:", root.mediaData.length)
            }

            

            loginManager.fetchMediaMetadata()

            
        }
    }

    function getMediaProgress(mediaId) {
        if (!mediaMetadata || !mediaId) return 0;
    
        const meta = mediaMetadata[mediaId];
        if (meta && meta.percentage_watched) {
            const percentage = parseFloat(meta.percentage_watched);
            return isNaN(percentage) ? 0 : percentage;
        }
        return 0;
    }
}