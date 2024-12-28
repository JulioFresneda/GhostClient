import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Controls.Basic
import QtMultimedia
import QtQuick.VectorImage
import com.ghoststream 1.0
import "." 

Item {
    id: root
    width: 1920
    height: 1080


    
    property bool isPlayerVisible: false
    property var currentMediaId


    Navigator {
        id: navigator
        onFilteredDataChanged: console.log("Filtered data updated")
    }

    Connections {
        target: filterBar
        function onFiltersChanged() {
            navigator.updateFilteredData()
        }
    }
    Connections {
        target: loginManager

        function onMediaMetadataFetched(metadata) {
            navigator.setMediaMetadata(metadata)
            console.log("Loaded media metadata:", navigator.mediaMetadata);
            
        }

        function onMediaDataFetched(fetchedData) {
            console.log("Received media data:", JSON.stringify(fetchedData, null, 2))

            if (fetchedData.collections) {
                navigator.setCollectionsData(fetchedData.collections)
            }

            if (fetchedData.media) {
                navigator.setMediaData(fetchedData.media)
            }

            

            loginManager.fetchMediaMetadata()

            
        }
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
        property string midnightBlue: "#191970"
        property string periwinkle: "#CCCCFF"
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
        visible: !isPlayerVisible || true
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
                    Layout.preferredHeight: parent.height * 0.02 // Takes 3/4 of the height
                }

                Button {
                    id: imageButton
                    z: 1
                    Layout.fillWidth: true // Match the width of Repeater buttons
                    height: width * 0.8    // Maintain the aspect ratio (adjust 0.6 to match your image's ratio)

                    background: Rectangle {
                        anchors.fill: parent
                        color: "transparent"

                        Image {
                            anchors.fill: parent
                            fillMode: Image.PreserveAspectFit // Maintain image proportions
                            source: imageButton.hovered ? "qrc:/media/logo_green.png" : "qrc:/media/logo.png"
                            smooth: true
                        }
                    }

                    onClicked: {
                        Qt.openUrlExternally("https://www.github.com/JulioFresneda/GhostClient");
                    }
                }
                Item {
                    Layout.preferredHeight: parent.height * 0.15 // Takes 3/4 of the height
                }

                // Categories
                Repeater {
                    model: navigator.sidebarCategories() // Call the C++ method
                    delegate: ItemDelegate {
                        Layout.fillWidth: true
                        height: 48

                        background: Rectangle {
                            color: navigator.currentCategory === modelData.key ? 
                                  colors.primary : "black"
                            radius: 8
                        }

                        contentItem: Text {
                            text: modelData.value // Use the value from the map
                            color: navigator.currentCategory === modelData.key ? 
                                  colors.textPrimary : colors.textSecondary
                            font.pointSize: 14
                            verticalAlignment: Text.AlignVCenter
                        }

                        onClicked: {
                            navigator.currentCategory = modelData.key
                            navigator.selectedCollectionId = ""
                            
                            
                        }
                    }
                }


                

                ItemDelegate {
                    Layout.fillWidth: true
                    height: 0
                    visible: navigator.selectedCollectionId !== ""

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
                        navigator.selectedCollectionId = ""
                        
                    }
                }
                Item { height: 60 } // Spacer
                ItemDelegate {
                    Layout.fillWidth: true
                    height: 0
                    visible: true

                    background: Rectangle {
                        //
                        color: "black"
                        //radius: 8
                        //border.width: 4
                        //border.color: colors.strongWhite
                    }

                    contentItem: RowLayout {
                        spacing: 0
                        Text {
                            text: "Salir"
                            color: colors.textSecondary
                            font.pointSize: 16
                        }
                    }

                    onClicked: {
                        Qt.quit()
                        
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

                        onTextChanged: navigator.updateFilteredData(searchField.text)
                    }

                    Item { Layout.fillWidth: true }

                    Text {
                        text: "Agrupar"
                        color: "white"  // White text color
                        anchors.verticalCenter: parent.verticalCenter 
                        font.pointSize: 14
                        visible: navigator.currentCategory === "movies"
                    }
                    CheckBox {
                        id: groupMoviesCheck
                        text: ""
                        
                        visible: navigator.currentCategory === "movies"
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

                        onCheckedChanged: {
                            navigator.updateFilteredData()
                            navigator.groupByCollection = groupMoviesCheck.checked
                        }
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
                visible: navigator.selectedCollectionId === "" ? true : false
                z: 2
                property int padding: 8
                // Properties to store selected filters


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

                    Connections {
                        target: navigator
                        onMediaLoaded: {
                            genreFilter.model = navigator.getUniqueGenres();
                            producerFilter.model = navigator.getUniqueProducers();
                        }
                    }
                    // Genres ComboBox
                    ComboBox {
                        id: genreFilter
                        Layout.preferredWidth: 150
                        model: navigator.getUniqueGenres()

                        
                        textRole: "text"
                        displayText: navigator.selectedGenres.length > 0 ?
                                     navigator.selectedGenres.join(", ") : "Genre"

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
                            checked: navigator.selectedGenres.includes(modelData.text)
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
                                    if (!navigator.selectedGenres.includes(modelData.text)) {
                                        navigator.selectedGenres.push(modelData.text)
                                    }
                                } else {
                                    const index = navigator.selectedGenres.indexOf(modelData.text)
                                    if (index > -1) {
                                        navigator.selectedGenres.splice(index, 1)
                                    }
                                }
                                console.log(filterBar.selectedGenres)
                                navigator.updateFilteredData()
                            }
                        }
                    }


                    // Era ComboBox
                    ComboBox {
                        id: eraFilter
                        Layout.preferredWidth: 120
                        model: ["60's", "70's", "80's", "90's", "2000's", "2010's", "2020's"]
                        displayText: navigator.selectedEras.length > 0 ?
                                     navigator.selectedEras.join(", ") : "Era"

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
                            checked: navigator.selectedEras.includes(modelData)
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
                                    if (!navigator.selectedEras.includes(modelData)) {
                                        navigator.selectedEras.push(modelData)
                                    }
                                } else {
                                    const index = navigator.selectedEras.indexOf(modelData)
                                    if (index > -1) {
                                        navigator.selectedEras.splice(index, 1)
                                    }
                                }
                                console.log(navigator.selectedEras)
                                navigator.updateFilteredData()
                            }
                        }

                        //closePolicy: Popup.CloseOnEscape // Keep the popup open for multiple selections
                    }


                    // Top Rated Switch
                    Switch {
                        id: topRatedSwitch
                        text: "<font color=\"white\">Top Rated (8+ IMBD)</font>"
                        checked: navigator.showTopRated
                        background: Rectangle {
                            color: colors.background
                            radius: 4
                        }
                        Layout.alignment: Qt.AlignVCenter
                        implicitHeight: 40
                        onCheckedChanged: {
                            navigator.showTopRated = checked
                            navigator.updateFilteredData()
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
                        model: navigator.getUniqueProducers()
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
                            navigator.selectedProducer = currentText === "All" ? "" : currentText
                            navigator.updateFilteredData()
                        }
                    }

                    ComboBox {
                        id: sortByComboBox
                        Layout.preferredWidth: 250
                        background: Rectangle {
                            color: colors.background
                            radius: 4
                        }
                        Layout.alignment: Qt.AlignVCenter
                        implicitHeight: 40
                        model: ["Title", "Year", "Rating"]
                        displayText: currentText || "Sort By"
                        contentItem: Item {
                            implicitWidth: sortText.implicitWidth
                            implicitHeight: sortText.implicitHeight

                            Text {
                                id: sortText
                                anchors {
                                    left: parent.left
                                    right: parent.right
                                    verticalCenter: parent.verticalCenter
                                    leftMargin: 12
                                    rightMargin: 8
                                }
                                text: sortByComboBox.displayText
                                color: "white"
                                verticalAlignment: Text.AlignVCenter
                                elide: Text.ElideRight
                                font.pointSize: 12
                            }
                        }
                        



                           
                        onActivated: {
                            navigator.sortBy = sortByComboBox.currentText === "Sort By" ? "title" : sortByComboBox.currentText
                            //navigator.updateFilteredData()
                        }
                    }

                    Button {
                        id: sortOrder
                        flat: true
                        Layout.preferredWidth: 24
                        Layout.preferredHeight: 24

                        contentItem: VectorImage {
                            source: navigator.sortOrder ? "qrc:/media/buttons/sort_asc.svg" : "qrc:/media/buttons/sort_desc.svg"
                            anchors.fill: parent // Make the VectorImage fill the Button
                        }

                        background: Rectangle {
                            color: "transparent"
                        }

                        onClicked: {
                            navigator.sortOrder = !navigator.sortOrder; // Toggle the custom property
                            console.log(navigator.sortBy)
                        }
                    }

                    Item {
                        width: 12 // Space to the left of the ComboBox
                    }

                    // Clear Filters Button
                    Button {
                        text: "Clear Filters"
                        flat: true
                        visible: true
                        onClicked: {
                            navigator.clearFilters()
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
                id: bgrectangle
                Layout.fillWidth: true
                Layout.fillHeight: true
                color: "#050505"
                radius: 8
                clip: true
                z: 1
                Layout.preferredHeight: parent.height - 360

                Repeater {
                    model: 5
                    Image {
                        id: ghost
                        // Make sure your prefix is correct. For example: "qrc:/media/bgghost.png"
                        source: "qrc:/media/bgghost.png"
                        // Position it initially (for example, horizontally centered, vertically centered)
                        x: Math.random() * (2000);
                        y: Math.random() * (200) + (index-1)*200;
                        //z: 100
                        width: 100
                        height: 100
                        sourceSize.width: 100
                        sourceSize.height: 100
                        property bool flipped: false // Tracks whether the image is flipped
                        transform: [Scale{ 
                            origin.x: ghost.width / 2
                            origin.y: ghost.height / 2
                            xScale: 1 }]
                        // Random horizontal movement
                        property bool allowMovement: true
                        property var randomduration: Math.floor(Math.random() * (2000)) + 1000;

                        // When x changes, smoothly animate
                        Behavior on x {
                            NumberAnimation {
                                duration: Math.floor(Math.random() * (30000 - 5000 + 1)) + 5000;
                                easing.type: Easing.InOutQuad
                            }
                        }

                        // Periodically choose a random X position
                        Timer {
                            interval: Math.floor(Math.random() * (10000 - 5000 + 1)) + 5000;
                            running: true
                            repeat: true
                            onTriggered: {
                                if (ghost.allowMovement) {
                                    // Move the ghost to a random horizontal position
                                    var newX = Math.random() * (bgrectangle.x + bgrectangle.width);

                                    // Check if the ghost is moving left or right
                                    if (newX < ghost.x && !ghost.flipped) {
                                        ghost.flipped = true; // Flip left
                                        ghost.transform[0].xScale = -1
                                        console.log("Flip to left")
                                    
                                    } else if (newX > ghost.x && ghost.flipped) {
                                        ghost.flipped = false; // Flip right
                                        console.log("Flip to right")
                                        ghost.transform[0].xScale = 1
                                    }

                                    // Move to the new random position
                                    ghost.x = newX;
                                
                                    console.log("New ghost pos: " + ghost.x + " " + ghost.y)
                                    console.log("jeje " + bgrectangle.y + " " + bgrectangle.height)
                                }
                            }
                        }

                        // Simple up/down "float" animation
                        // This will move the ghost from y to (y - 30) and back in a loop
                        SequentialAnimation {
                            id: floatAnimation
                            loops: Animation.Infinite
                            running: true
                            

                            NumberAnimation {
                                target: ghost
                                property: "y"
                                duration: ghost.randomduration
                                easing.type: Easing.InOutQuad
                                from: ghost.y
                                to: ghost.y - 30
                            }
                            NumberAnimation {
                                target: ghost
                                property: "y"
                                duration: ghost.randomduration
                                easing.type: Easing.InOutQuad
                                from: ghost.y - 30
                                to: ghost.y
                            }
                        }
                    }
                }

                GridView {
                    id: mediaGrid
                    anchors.fill: parent
                    cellWidth: 200
                    cellHeight: 300
                    model: navigator.filteredData

                    delegate: MediaCard {
                        width: 180
                        height: 280
                        title: navigator.selectedCollectionId ? 
                               (modelData.title || "") : // Title is currentCat is not series or movies
                               (navigator.currentCategory === "series" || // If serie, or movies and grouped and coll title exists, colltitle
                               (navigator.currentCategory === "movies" && groupMoviesCheck.checked && modelData.collection_title)) ?
                               modelData.collection_title || "" :
                               modelData.title || ""
                        season: (modelData.season || "")
                        episode: (modelData.episode || "")
                        mediaId: modelData.ID || ""
                        backupId: navigator.getCollectionId(modelData.ID)
                        property bool isCollection: modelData.collection_title ? true : false
                    }
                }

                
            }
        }
    }

    Rectangle {
        id: mainContainer
        anchors.fill: parent
        color: "transparent"
        visible: isPlayerVisible
        property bool animationComplete: false
        // A child that can move freely along the x-axis
        Rectangle {
            id: transitionbg
            // Anchor only top and bottom, so height matches the parent
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            width: parent.width*2  // So it matches the parent's width
            color: "transparent"

            // Start off-screen to the left
            x: -width -100

            Image {
                anchors.fill: parent
                id: transitionbg_image
                source: "qrc:/media/transitionbg.png"
                width: 3840
                height: 1080
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.verticalCenter: parent.verticalCenter

                // Optional: Make it look smoother when scaled
                smooth: true
            }

            // Smooth animation whenever x changes
            Behavior on x {
                NumberAnimation {
                    duration: 5000
                    easing.type: Easing.InOutQuad
                    
                }
            }

            // For example, let's say you toggle `visible` in QML.
            // We can slide in/out on "visible" changes:
            onVisibleChanged: {
                if (visible) {
                    transitionbg.x = 0;       // Slide fully onscreen
                }
            }
            onXChanged: {
                if (x === 0) {
                    console.log("Animation reached target value!");
                    mainContainer.animationComplete = true
                }
            }
        }
    }


    Rectangle {
        id: playerContainer
        anchors.fill: parent
        visible: isPlayerVisible && mainContainer.animationComplete
        z: isPlayerVisible ? 100 : -1

        Loader {
            id: playerLoader
            anchors.fill: parent
            active: isPlayerVisible && mainContainer.animationComplete
            onLoaded: {
                console.log(navigator.mediaMetadata[currentMediaId])
                navigator.selectedCollectionId = navigator.getCollectionId(currentMediaId)
            }
            sourceComponent: Component {
                MediaPlayer {
                    mediaId: currentMediaId
                    title: navigator.getMediaTitle(currentMediaId)
                    mediaMetadata: navigator.getMediaMetadata(currentMediaId)
                    episodeType: navigator.getEpisodeType(currentMediaId)
                    
                    onCloseRequested: {
                        isPlayerVisible = false
                        currentMediaId = ""
                        //navigator.update
                    }
                    onMediaEnded: {
                        mediaId = navigator.getNextEpisode(mediaId, 1)
                        mediaMetadata = navigator.getMediaMetadata(mediaId)
                    }
                    onNextEpisode: {
                        currentMediaId = navigator.getNextEpisode(mediaId, 1)
                        mediaId = currentMediaId
                        mediaMetadata = navigator.getMediaMetadata(mediaId)
                    }
                    onLastEpisode: {
                        currentMediaId = navigator.getNextEpisode(mediaId, -1)
                        mediaId = currentMediaId
                        mediaMetadata = navigator.getMediaMetadata(mediaId)
                    }
                    
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
        property string backupId: ""
        signal clicked
        color: "black"
        border.width: 5
        bottomLeftRadius: 0
        bottomRightRadius: 0
        border.color: cardMouseArea.containsMouse ? colors.green : colors.strongWhite

        //gradient: Gradient {
            //GradientStop { position: 0.0; color: colors.strongWhite }
        //    GradientStop { position: 0.84; color: colors.strongWhite }
        //    GradientStop { position: 0.88; color: "white" }
        //}

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
                    backupId: card.backupId
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
                    visible: navigator.getMediaProgress(card.mediaId) > 0
    
                    Rectangle {
                        id: progressFill
                        anchors {
                            left: parent.left
                            top: parent.top
                            bottom: parent.bottom
                        }
                        width: parent.width * navigator.getMediaProgress(card.mediaId)
                        gradient: Gradient {
                            orientation: Gradient.Horizontal
                            GradientStop {
                                position: 1.0
                                color: navigator.getMediaProgress(card.mediaId) > 0.99 ? colors.midnightBlue : colors.superGreen
                            }
                            GradientStop {
                                position: navigator.getMediaProgress(card.mediaId) > 0.99 ? 0.5 : 0.8
                                color: navigator.getMediaProgress(card.mediaId) > 0.99 ? colors.periwinkle : colors.green
                            }
                            GradientStop {
                                position: 0.0
                                color: navigator.getMediaProgress(card.mediaId) > 0.99 ? colors.midnightBlue : colors.green
                            }
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
                            height: seasonText.implicitHeight + 8 // Add padding inside the Rectangle
                            color: colors.strongWhite // Set the background color
                            //gradient: Gradient {
                            //    orientation: Gradient.Horizontal
                            //    GradientStop { position: 0.0; color: colors.strongWhite }
                            //    GradientStop { position: 1; color: colors.strongWhite }
                            //    GradientStop { position: 0.1; color: "black" }
                            //    GradientStop { position: 0.9; color: "black" }
                            //}
                            //radius: 4

                            Text {
                                id: seasonText
                                text: "Season " + season + " - Episode " + episode
                                color: "black" // White text color
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
                            color: "white"
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
                if (navigator.selectedCollectionId) {
                    console.log("Loading media from collection:", modelData.ID)
                    currentMediaId = modelData.ID
                    isPlayerVisible = true
                } else if (modelData.collection_title) {
                    navigator.selectedCollectionId = modelData.ID
                    selectedCollectionTitle = modelData.collection_title
                    navigator.updateFilteredData()
                } 
                else {
                    console.log("Loading standalone media:", modelData.ID)
                    currentMediaId = modelData.ID
                    isPlayerVisible = true
                }
            }
        }
    }

    // Component for list items
    

    function formatTime(ms) {
        let seconds = Math.floor(ms / 1000)
        let minutes = Math.floor(seconds / 60)
        seconds = seconds % 60
        return minutes.toString().padStart(2, '0') + ':' + 
               seconds.toString().padStart(2, '0')
    }
//searchField.text.toLowerCase()
    

    function showMediaDetails(mediaItem) {
        console.log("Selected media:", JSON.stringify(mediaItem, null, 2))
        currentMediaId = mediaItem.ID
        isPlayerVisible = true
    }

    

    

    
}