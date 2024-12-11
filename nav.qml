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
    property string selectedCollectionId: ""
    property string selectedCollectionTitle: ""
    property bool isPlayerVisible: false
    property string currentMediaId: ""

    MediaFilterHandler {
        id: filterHandler
    }

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
        property string superGreen: "#66f557"
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
            Layout.preferredWidth: parent.height / 4.5
            Layout.fillHeight: true
            color: colors.surface
            
            Image {
                z: 0
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
                                  colors.primary : "black"
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
                            selectedCollectionId = ""
                            selectedCollectionTitle = ""
                        
                            filterContent()
                        }
                    }
                }

                

                ItemDelegate {
                    Layout.fillWidth: true
                    height: 0
                    visible: selectedCollectionId !== ""

                    background: Rectangle {
                        //
                        color: "black"
                        radius: 8
                        border.width: 4
                        border.color: colors.strongWhite
                    }

                    contentItem: RowLayout {
                        spacing: 0
                        Text {
                            text: " ←"
                            color: colors.textSecondary
                            font.pointSize: 16
                        }
                        Text {
                            text: "Back"
                            color: colors.textSecondary
                            font.pointSize: 14
                        }
                    }

                    onClicked: {
                        selectedCollectionId = ""
                        selectedCollectionTitle = ""
                        filterContent()
                    }
                }
                Item { Layout.fillHeight: true } // Spacer
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
                z: 2

                RowLayout {
                    Layout.fillWidth: true
                    anchors.fill: parent
                    anchors.margins: 8
                    spacing: 16

                    //Item { Layout.fillWidth: true }

                    TextField {
                        id: searchField
                        Layout.preferredWidth: 300  // Set a width if you need to control the width explicitly
                        //height: 40  // Height should be less than container height to allow centering
                        anchors.verticalCenter: parent.verticalCenter  // Vertically center the TextField
                        placeholderText: "Search your ghost..."
                        placeholderTextColor: colors.strongWhite
                        color: colors.strongWhite
                        font.pointSize: 12

                        background: Rectangle {
                            color: colors.background
                            radius: 4
                        }

                        onTextChanged: filterContent()
                    }

                    Item { Layout.fillWidth: true }

                    Text {
                        text: "Agrupar"
                        color: "white"  // White text color
                        anchors.verticalCenter: parent.verticalCenter 
                        font.pointSize: 14
                        visible: currentCategory === "movies"
                    }
                    CheckBox {
                        id: groupMoviesCheck
                        text: ""
                        
                        visible: currentCategory === "movies"
                        checked: true
                        
                        contentItem: Text {
                            text: groupMoviesCheck.text
                            color: colors.textPrimary
                            leftPadding: groupMoviesCheck.indicator.width + 4
                            verticalAlignment: Text.AlignVCenter
                        }

                        indicator: Rectangle {
                            implicitWidth: 40
                            implicitHeight: 40
                            radius: 3
                            border.color: colors.primary
                            color: "transparent"

                            Rectangle {
                                width: 20
                                height: 20
                                anchors.centerIn: parent
                                radius: 2
                                color: colors.primary
                                visible: groupMoviesCheck.checked
                            }
                        }

                        onCheckedChanged: filterContent()
                    }

                    
                }
            }

            // After the search bar and before the content grid
            // FilterBar.qml
            Rectangle {
                id: filterBar
                Layout.fillWidth: true
                Layout.preferredHeight: filterBarRowLayout.implicitHeight + (2 * padding)
                color: colors.surface
                radius: 8
                visible: selectedCollectionId === "" ? true : false
                z: 2
                property int padding: 8
                // Properties to store selected filters
                property var selectedGenres: []
                property var selectedEras: []
                property bool showTopRated: false
                property string selectedDirector: ""
                property string selectedProducer: ""

                // Signals for filter changes
                signal filtersChanged()

                RowLayout {
                    id: filterBarRowLayout
                    Layout.fillWidth: true
                    Layout.alignment: Qt.AlignVCenter
                    anchors {
                        left: parent.left
                        right: parent.right
                        verticalCenter: parent.verticalCenter
                        leftMargin: filterBar.padding
                        rightMargin: filterBar.padding
                    }
                    
                    Item {
                        width: 4 // Space to the left of the ComboBox
                    }
                    // Genres ComboBox
                    ComboBox {
                        id: genreFilter
                        Layout.preferredWidth: 150
                        model: filterHandler.getUniqueGenres(collectionsData, mediaData)
                        textRole: "text"
                        displayText: filterBar.selectedGenres.length > 0 ?
                                     filterBar.selectedGenres.join(", ") : "Genre"

                        background: Rectangle {
                            color: colors.background
                            radius: 4
                        }
                        Layout.alignment: Qt.AlignVCenter
                        implicitHeight: 40
                        Item {
                            width: 4 // Space to the left of the ComboBox
                        }
                        contentItem: Item {
                            implicitWidth: genreText.implicitWidth
                            implicitHeight: genreText.implicitHeight

                            Text {
                                id: genreText
                                anchors {
                                    left: parent.left
                                    right: parent.right
                                    verticalCenter: parent.verticalCenter
                                    leftMargin: 12  // Increased left padding
                                    rightMargin: 8  // Added right padding
                                }
                                text: genreFilter.displayText
                                color: "white"
                                verticalAlignment: Text.AlignVCenter
                                elide: Text.ElideRight
                                font.pointSize: 12
                            }
                        }
                        
                        delegate: CheckDelegate {
                            width: parent.width
                            text: modelData.text
                            checked: filterBar.selectedGenres.includes(modelData.text)
                            background: Rectangle {
                                anchors.fill: parent
                                color: checked ? colors.background : colors.background // Background color for checked/unchecked items
                            }
                            contentItem: Text {
                                anchors {
                                    left: parent.left
                                    verticalCenter: parent.verticalCenter
                                    leftMargin: 12
                                }
                                text: modelData.text
                                color: "white" // Change text color when the item is checked
                                font.pointSize: 12
                            }
                            
                            onCheckedChanged: {
                                if (checked) {
                                    if (!filterBar.selectedGenres.includes(modelData.text)) {
                                        filterBar.selectedGenres.push(modelData.text)
                                    }
                                } else {
                                    const index = filterBar.selectedGenres.indexOf(modelData.text)
                                    if (index > -1) {
                                        filterBar.selectedGenres.splice(index, 1)
                                    }
                                }
                                console.log(filterBar.selectedGenres)
                                filterBar.filtersChanged()
                            }
                        }
                    }


                    // Era ComboBox
                    ComboBox {
                        id: eraFilter
                        Layout.preferredWidth: 120
                        model: ["60's", "70's", "80's", "90's", "2000's", "2010's", "2020's"]
                        displayText: filterBar.selectedEras.length > 0 ?
                                     filterBar.selectedEras.join(", ") : "Era"

                        background: Rectangle {
                            color: colors.background
                            radius: 4
                        }
                        Layout.alignment: Qt.AlignVCenter
                        implicitHeight: 40
                        Item {
                            width: 4 // Space to the left of the ComboBox
                        }
                        contentItem: Item {
                            implicitWidth: eraText.implicitWidth
                            implicitHeight: eraText.implicitHeight

                            Text {
                                id: eraText
                                anchors {
                                    left: parent.left
                                    right: parent.right
                                    verticalCenter: parent.verticalCenter
                                    leftMargin: 12  // Increased left padding
                                    rightMargin: 8  // Added right padding
                                }
                                text: eraFilter.displayText
                                color: "white"
                                verticalAlignment: Text.AlignVCenter
                                elide: Text.ElideRight
                                font.pointSize: 12
                            }
                        }

                        delegate: CheckDelegate {
                            width: parent.width
                            text: modelData
                            checked: filterBar.selectedEras.includes(modelData)
                            background: Rectangle {
                                anchors.fill: parent
                                color: checked ? colors.background : colors.background // Background color for checked/unchecked items
                            }
                            contentItem: Text {
                                anchors {
                                    left: parent.left
                                    verticalCenter: parent.verticalCenter
                                    leftMargin: 12
                                }
                                text: modelData
                                color: "white" // Change text color when the item is checked
                                font.pointSize: 12
                            }

                            onCheckedChanged: {
                                if (checked) {
                                    if (!filterBar.selectedEras.includes(modelData)) {
                                        filterBar.selectedEras.push(modelData)
                                    }
                                } else {
                                    const index = filterBar.selectedEras.indexOf(modelData)
                                    if (index > -1) {
                                        filterBar.selectedEras.splice(index, 1)
                                    }
                                }
                                console.log(filterBar.selectedEras)
                                filterBar.filtersChanged()
                            }
                        }

                        //closePolicy: Popup.CloseOnEscape // Keep the popup open for multiple selections
                    }


                    // Top Rated Switch
                    Switch {
                        id: topRatedSwitch
                        text: "<font color=\"white\">Top Rated (8+ IMBD)</font>"
                        checked: filterBar.showTopRated
                        background: Rectangle {
                            color: colors.background
                            radius: 4
                        }
                        Layout.alignment: Qt.AlignVCenter
                        implicitHeight: 40
                        onCheckedChanged: {
                            filterBar.showTopRated = checked
                            filterBar.filtersChanged()
                        }
                    }

                    // Producer ComboBox
                    ComboBox {
                        id: producerFilter
                        Layout.preferredWidth: 250
                        background: Rectangle {
                            color: colors.background
                            radius: 4
                        }
                        Layout.alignment: Qt.AlignVCenter
                        implicitHeight: 40
                        model: filterHandler.getUniqueProducers(collectionsData, mediaData)
                        displayText: currentText || "Producer"
                        contentItem: Item {
                            implicitWidth: producerText.implicitWidth
                            implicitHeight: producerText.implicitHeight

                            Text {
                                id: producerText
                                anchors {
                                    left: parent.left
                                    right: parent.right
                                    verticalCenter: parent.verticalCenter
                                    leftMargin: 12
                                    rightMargin: 8
                                }
                                text: producerFilter.displayText
                                color: "white"
                                verticalAlignment: Text.AlignVCenter
                                elide: Text.ElideRight
                                font.pointSize: 12
                            }
                        }
                        



                           
                        onActivated: {
                            filterBar.selectedProducer = currentText === "All" ? "" : currentText
                            filterBar.filtersChanged()
                        }
                    }

                    // Clear Filters Button
                    Button {
                        text: "Clear Filters"
                        flat: true
                        visible: true
                        onClicked: {
                            filterBar.selectedGenres = []
                            filterBar.selectedEras = []
                            filterBar.showTopRated = false
                            filterBar.selectedDirector = "All"
                            filterBar.selectedProducer = "All"
                        }

                        contentItem: Text {
                            text: parent.text
                            color: colors.textSecondary
                            font.pointSize: 12
                        }

                        background: Rectangle {
                            color: parent.hovered ? Qt.darker(colors.surface, 1.2) : "transparent"
                            radius: 4
                        }
                    }

                    Item { Layout.fillWidth: true } // Spacer
                }

                

                

                

                

                
            }

            // Content Grid/List View
            Rectangle {
                Layout.fillWidth: true
                Layout.fillHeight: true
                color: colors.surface
                radius: 8
                clip: true
                z: 1
                Layout.preferredHeight: parent.height - 360

                GridView {
                    id: mediaGrid
                    anchors.fill: parent
                    cellWidth: 200
                    cellHeight: 300
                    model: filteredData

                    delegate: MediaCard {
                        width: 180
                        height: 280
                        title: selectedCollectionId ? 
                               (modelData.title || "") :
                               (currentCategory === "series" || 
                               (currentCategory === "movies" && groupMoviesCheck.checked && modelData.collection_title)) ?
                               modelData.collection_title || "" :
                               modelData.title || ""
                        season: (modelData.season || "")
                        episode: (modelData.episode || "")
                        mediaId: modelData.ID || ""
                        property bool isCollection: modelData.collection_title ? true : false
                    }
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
            onLoaded: {
                // Connect the child's signal to a function in the parent
                playerLoader.item.nextEpisode.connect(playerContainer.onNextEpisode)
            }
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
        function onNextEpisode(){
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
        function onLastEpisode(){
            if (selectedCollectionId) {
                // Find current media in filtered data
                let currentIndex = filteredData.findIndex(media => media.ID === currentMediaId)
                if (currentIndex >= 1 && currentIndex < filteredData.length) {
                    // Play next episode
                    currentMediaId = filteredData[currentIndex - 1].ID
                    mediaId = currentMediaId
                    mediaMetadata = root.mediaMetadata[currentMediaId]
                }
            }
        }

        
    }

    // Components for grid and list items
    component MediaCard: Rectangle {
        id: card
        property string title: ""
        property string season: ""
        property string episode: ""
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
                height: season === "" ? titleText.height + 16 : titleText.height + 50
                color: "transparent"
                Rectangle {
                    id: progressBar
                    anchors {
                        left: parent.left
                        right: parent.right
                        bottom: parent.top
                    }
                    height: 6
                    color: colors.background
                    visible: getMediaProgress(card.mediaId) > 0
    
                    Rectangle {
                        id: progressFill
                        anchors {
                            left: parent.left
                            top: parent.top
                            bottom: parent.bottom
                        }
                        width: parent.width * getMediaProgress(card.mediaId)
                        gradient: Gradient {
                                    orientation: Gradient.Horizontal
                                    GradientStop { position: 1.0; color: colors.superGreen }
                                    GradientStop { position: 0.8; color: colors.green }
                                    GradientStop { position: 0.0; color: colors.green }
                                }
                    }
                }
                
                Item {
                    width: parent.width
                    // Dynamically adjust height to fit both texts with spacing
                    height: (season !== "" ? seasonText.implicitHeight + 12 : 0) + titleText.implicitHeight + 18 

                    Column {
                        anchors.fill: parent
                        spacing: 6 // Space between the two text elements
                        anchors.margins: season !== "" ? 0 : 6

                        Rectangle {
                            visible: season !== "" // Show only if season is not empty
                            width: parent.width
                            height: seasonText.implicitHeight + 12 // Add padding inside the Rectangle
                            color: "black" // Set the background color
                            //radius: 4

                            Text {
                                id: seasonText
                                text: "Season " + season + " - Episode " + episode
                                color: "white" // White text color
                                font.pointSize: 10
                                horizontalAlignment: Text.AlignHCenter
                                verticalAlignment: Text.AlignVCenter
                                elide: Text.ElideRight
                                wrapMode: Text.WordWrap
                                anchors {
                                    left: parent.left
                                    right: parent.right
                                    verticalCenter: parent.verticalCenter
                                    margins: 6 // Add margins to prevent text from touching the edges
                                }
                            }
                        }

                        Text {
                            id: titleText
                            text: title
                            color: "black"
                            font.pointSize: 12
                            horizontalAlignment: Text.AlignHCenter
                            elide: Text.ElideRight
                            wrapMode: Text.WordWrap
                            width: parent.width - 12 // Explicit width with margin for wrapping
                        }
                    }
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
        let filteredResults = []
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

        filteredResults = filteredData
        if (filterBar.selectedGenres.length > 0) {
            console.log(filterBar.selectedGenres)
            filteredResults = filterHandler.filterByGenre(filteredResults, filterBar.selectedGenres)
        }

        if (filterBar.showTopRated) {
            filteredResults = filterHandler.filterByRating(filteredResults, 8.0)
        }

        if (filterBar.selectedEras.length > 0) {
            filteredResults = filterHandler.filterByEra(filteredResults, filterBar.selectedEras)
        }

        if (filterBar.selectedProducer && filterBar.selectedProducer !== "All") {
            filteredResults = filterHandler.filterByProducer(filteredResults, filterBar.selectedProducer)
        }
        filteredData = filteredResults

        console.log("Filtered results:", filteredData.length, "items")
    }

    function showMediaDetails(mediaItem) {
        console.log("Selected media:", JSON.stringify(mediaItem, null, 2))
        currentMediaId = mediaItem.ID
        isPlayerVisible = true
    }

    

    Connections {
        target: filterBar
        function onFiltersChanged() {
            filterContent()
        }
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