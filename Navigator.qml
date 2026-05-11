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
    property string token: ""

    // Details overlay state — populated by MediaCard info-button clicks.
    property bool isDetailsVisible: false
    property var detailsData: null
    property string detailsCoverId: ""
    property string detailsCoverBackup: ""

    // Holds the next category while the fade-out plays, so the actual
    // navigator.selectCategory() call happens at the bottom of the dip
    // (mediaGrid opacity = 0) and the content swap is invisible.
    property string _pendingCategory: ""

    function switchCategory(cat) {
        // No-op if nothing would change — avoid pointless fades.
        if (cat === navigator.currentCategory && navigator.selectedCollectionId === "") return
        _pendingCategory = cat
        categorySwitchFade.restart()
    }

    SequentialAnimation {
        id: categorySwitchFade
        NumberAnimation { target: mediaGrid; property: "opacity"; to: 0; duration: 120; easing.type: Easing.OutQuad }
        ScriptAction { script: navigator.selectCategory(root._pendingCategory, "") }
        NumberAnimation { target: mediaGrid; property: "opacity"; to: 1; duration: 180; easing.type: Easing.InQuad }
    }

    // Emitted when the user wants to switch profile — main.qml hides the
    // navigator and shows the profile selector again.
    signal backToProfileSelect

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
                id: sidebarImage
                source: "qrc:/media/sidebar.png"
            }

            

            ColumnLayout {
                id: sidebar
                anchors.fill: parent
                anchors.margins: 16
                spacing: 24
                property bool usingit: true
                Keys.onUpPressed: {
                    var prev = navigator.getPreviousCategory()
                    root.switchCategory(prev != "" ? prev : navigator.currentCategory)
                }
                Keys.onDownPressed: {
                    var next = navigator.getNextCategory()
                    root.switchCategory(next != "" ? next : navigator.currentCategory)
                }
                Keys.onRightPressed: {
                    usingit = false
                    mediaGrid.forceActiveFocus()
                    mediaGrid.moveToNextItem()
                }
                Keys.onBackPressed: {
                    if(navigator.selectedCollectionId == ""){
                        Qt.quit()
                    }
                    else {
                        navigator.selectedCollectionId = ""
                    }   
                }
                
                Component.onCompleted: {
                    sidebar.forceActiveFocus() // Ensure focus starts at this container
                }
                Item {
                    Layout.preferredHeight: parent.height * 0.02 // Takes 3/4 of the height
                }

                Button {
                    id: imageButton
                    z: 1
                    Layout.fillWidth: true
                    Layout.preferredHeight: width * 0.8

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
                        Qt.openUrlExternally("https://github.com/JulioFresneda/GhostStream");
                    }
                }
                Item {
                    Layout.preferredHeight: parent.height * 0.15 // Takes 3/4 of the height
                }

                // Categories
                Repeater {
                    model: navigator.sidebarCategories()
                    delegate: ItemDelegate {
                        Layout.fillWidth: true
                        height: 44
                        focus: index === sidebar.currentIndex
                        property bool isActive: navigator.currentCategory === modelData.key

                        background: Rectangle {
                            color: hovered ? "#14FFFFFF" : "transparent"
                            radius: 0

                            // Green active-state bar on the left edge
                            Rectangle {
                                visible: parent.parent.isActive
                                anchors.left: parent.left
                                anchors.top: parent.top
                                anchors.bottom: parent.bottom
                                width: 4
                                color: colors.superGreen
                            }
                        }

                        contentItem: Text {
                            text: modelData.value
                            color: parent.isActive ? colors.superGreen
                                   : (parent.hovered ? "white" : "#B3FFFFFF")
                            font.pointSize: 14
                            font.weight: parent.isActive ? Font.Medium : Font.Normal
                            leftPadding: 16
                            verticalAlignment: Text.AlignVCenter
                            Behavior on color { ColorAnimation { duration: 120 } }
                        }

                        onClicked: root.switchCategory(modelData.key)
                    }
                }

                // ── In-collection back button (visible only inside a collection)
                ItemDelegate {
                    id: backToLibraryDelegate
                    Layout.fillWidth: true
                    height: 44
                    visible: navigator.selectedCollectionId !== ""

                    background: Rectangle {
                        color: backToLibraryDelegate.hovered ? "#14FFFFFF" : "transparent"
                        radius: 0
                    }
                    contentItem: Text {
                        text: "←  Back to library"
                        color: parent.hovered ? "white" : "#B3FFFFFF"
                        font.pointSize: 13
                        leftPadding: 16
                        verticalAlignment: Text.AlignVCenter
                        Behavior on color { ColorAnimation { duration: 120 } }
                    }
                    onClicked: navigator.selectedCollectionId = ""
                }

                // Spacing between categories and the bottom group
                Item { Layout.preferredHeight: 24 }

                ItemDelegate {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 44

                    background: Rectangle {
                        color: parent.hovered ? "#14FFFFFF" : "transparent"
                        radius: 0
                    }
                    contentItem: Text {
                        text: "Switch profile"
                        color: parent.hovered ? "white" : "#B3FFFFFF"
                        font.pointSize: 14
                        font.weight: Font.Normal
                        leftPadding: 16
                        verticalAlignment: Text.AlignVCenter
                        Behavior on color { ColorAnimation { duration: 120 } }
                    }
                    onClicked: root.backToProfileSelect()
                }

                ItemDelegate {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 44

                    background: Rectangle {
                        color: parent.hovered ? "#33FF4444" : "transparent"
                        radius: 0
                    }
                    contentItem: Text {
                        text: "Exit"
                        color: parent.hovered ? "#FF6666" : "#B3FFFFFF"
                        font.pointSize: 14
                        font.weight: Font.Normal
                        leftPadding: 16
                        verticalAlignment: Text.AlignVCenter
                        Behavior on color { ColorAnimation { duration: 120 } }
                    }
                    onClicked: Qt.quit()
                }

                Item { Layout.fillHeight: true } // push everything up
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
                radius: 0
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
                        Layout.alignment: Qt.AlignVCenter  // Vertically center the TextField
                        placeholderText: "Search your ghost..."
                        placeholderTextColor: colors.strongWhite
                        color: colors.strongWhite
                        font.pointSize: 12

                        background: Rectangle {
                            color: colors.background
                            radius: 0
                            border.color: searchField.activeFocus ? colors.superGreen : "#22FFFFFF"
                            border.width: 1
                        }
                        leftPadding: 12
                        rightPadding: 12

                        onTextChanged: navigator.updateFilteredData(searchField.text)
                    }

                    Item { Layout.fillWidth: true }

                    Text {
                        text: "Group"
                        color: "white"  // White text color
                        Layout.alignment: Qt.AlignVCenter 
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
                            radius: 0
                            border.color: colors.primary
                            color: "transparent"

                            Rectangle {
                                width: 20
                                height: 20
                                anchors.centerIn: parent
                                radius: 0
                                color: colors.primary
                                visible: groupMoviesCheck.checked
                            }
                        }

                        onCheckedChanged: navigator.groupByCollection = groupMoviesCheck.checked
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
                radius: 0
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
                        function onMediaLoaded() {
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
                            radius: 0
                            border.color: genreFilter.activeFocus ? colors.superGreen : "#22FFFFFF"
                            border.width: 1
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
                                // QML JS arrays from QList<QString> properties
                                // must be reassigned to trigger the setter —
                                // mutating in place doesn't propagate.
                                let g = navigator.selectedGenres.slice()
                                if (checked) {
                                    if (!g.includes(modelData.text)) g.push(modelData.text)
                                } else {
                                    const i = g.indexOf(modelData.text)
                                    if (i > -1) g.splice(i, 1)
                                }
                                navigator.selectedGenres = g
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
                            radius: 0
                            border.color: eraFilter.activeFocus ? colors.superGreen : "#22FFFFFF"
                            border.width: 1
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
                                let e = navigator.selectedEras.slice()
                                if (checked) {
                                    if (!e.includes(modelData)) e.push(modelData)
                                } else {
                                    const i = e.indexOf(modelData)
                                    if (i > -1) e.splice(i, 1)
                                }
                                navigator.selectedEras = e
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
                            radius: 0
                            border.color: "#22FFFFFF"
                            border.width: 1
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
                            radius: 0
                            border.color: producerFilter.activeFocus ? colors.superGreen : "#22FFFFFF"
                            border.width: 1
                        }
                        Layout.alignment: Qt.AlignVCenter
                        implicitHeight: 40
                        model: navigator.getUniqueProducers()
                        // The model has "All" prepended as index 0 — show the
                        // placeholder until the user actually picks a producer.
                        displayText: (currentIndex > 0 && currentText) ? currentText : "Producer"
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
                            radius: 0
                            border.color: sortByComboBox.activeFocus ? colors.superGreen : "#22FFFFFF"
                            border.width: 1
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
                            // ComboBox displayText is driven by its internal
                            // currentIndex, not navigator.selectedProducer —
                            // so we have to reset it explicitly.
                            producerFilter.currentIndex = 0
                        }

                        contentItem: Text {
                            text: parent.text
                            color: colors.textSecondary
                            font.pointSize: 12
                        }

                        background: Rectangle {
                            color: parent.hovered ? Qt.darker(colors.surface, 1.2) : "transparent"
                            radius: 0
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
                radius: 0
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
                                        //console.log("Flip to left")
                                    
                                    } else if (newX > ghost.x && ghost.flipped) {
                                        ghost.flipped = false; // Flip right
                                        //console.log("Flip to right")
                                        ghost.transform[0].xScale = 1
                                    }

                                    // Move to the new random position
                                    ghost.x = newX;
                                
                                    //console.log("New ghost pos: " + ghost.x + " " + ghost.y)
                                    //console.log("jeje " + bgrectangle.y + " " + bgrectangle.height)
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
                    focus: false
                    currentIndex: -1
                    
                    delegate: MediaCard {
                        width: 180
                        height: 280
                        focus: GridView.isCurrentItem 
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

                        Keys.onReturnPressed: _activate()
                        Keys.onRightPressed: {
                            moveToNextItem()
                            //console.log(sidebar.usingit)
                        }
                        Keys.onLeftPressed: {
                            moveToPreviousItem()
                        }
                        Keys.onUpPressed: {
                            sidebar.usingit = true
                            mediaGrid.currentIndex = -1
                            sidebar.forceActiveFocus()
                            var prev = navigator.getPreviousCategory()
                            root.switchCategory(prev != "" ? prev : navigator.currentCategory)
                        }
                        Keys.onDownPressed: {
                            sidebar.usingit = true
                            mediaGrid.currentIndex = -1
                            sidebar.forceActiveFocus()
                            var next = navigator.getNextCategory()
                            root.switchCategory(next != "" ? next : navigator.currentCategory)
                        }

                        function moveToNextItem() {
                            if (mediaGrid.currentIndex < mediaGrid.count - 1) {
                                mediaGrid.currentIndex++
                                console.log("Moved to next item:", mediaGrid.currentIndex)
                            }
                        }

                        function moveToPreviousItem() {
                            if (mediaGrid.currentIndex > 0) {
                                mediaGrid.currentIndex--
                                console.log("Moved to previous item:", mediaGrid.currentIndex)
                            }
                        }
                    }
                    
                }

                
            }
        }
    }

    MediaDetails {
        id: mediaDetailsView
        anchors.fill: parent
        visible: isDetailsVisible
        z: 90
        entity: detailsData || ({})
        coverMediaId: detailsCoverId
        coverBackupId: detailsCoverBackup

        onCloseRequested: isDetailsVisible = false
        onPlayRequested: function(id) {
            isDetailsVisible = false
            currentMediaId = id
            isPlayerVisible = true
        }
        onOpenCollectionRequested: function(id) {
            isDetailsVisible = false
            navigator.selectedCollectionId = id
        }
    }

    Rectangle {
        id: playerContainer
        anchors.fill: parent
        color: "black"
        // Fade in/out so the cut to the player isn't abrupt; the MediaPlayer's
        // own loading screen (black + rotating loading.png) is what shows
        // during the fade, so the rotation acts as the transition image.
        opacity: isPlayerVisible ? 1 : 0
        visible: opacity > 0
        z: isPlayerVisible ? 100 : -1

        Behavior on opacity {
            NumberAnimation { duration: 400; easing.type: Easing.InOutQuad }
        }

        Loader {
            id: playerLoader
            anchors.fill: parent
            active: isPlayerVisible
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
        border.color: cardHover.hovered || (!sidebar.usingit && focus) ? colors.green : colors.strongWhite

        // Hover-only — never participates in click hit-testing, so the cover
        // and info-button MouseAreas below receive their events normally.
        HoverHandler {
            id: cardHover
        }

        // Activation = "play this card / enter this collection". Mirrors the
        // old whole-card onClicked logic so cover-clicks and Keys.onReturn
        // behave identically.
        function _activate() {
            if (navigator.selectedCollectionId) {
                currentMediaId = modelData.ID
                isPlayerVisible = true
            } else if (modelData.collection_title) {
                navigator.selectedCollectionId = modelData.ID
            } else {
                currentMediaId = modelData.ID
                isPlayerVisible = true
            }
        }

        function _showDetails() {
            detailsData = modelData
            detailsCoverId = mediaId
            detailsCoverBackup = backupId
            isDetailsVisible = true
        }

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
                    coverImageBase64: loginManager.getCoverImage(card.mediaId, card.backupId)
                }

                // Cover click → play directly (or enter the collection).
                MouseArea {
                    anchors.fill: parent
                    cursorShape: Qt.PointingHandCursor
                    onClicked: card._activate()
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
                    // Cache the progress value once per delegate render —
                    // getMediaProgress hits a map and we'd otherwise call it
                    // six times below (visible + width + 4 gradient stops).
                    property real progress: navigator.getMediaProgress(card.mediaId)
                    property bool completed: progress > 0.99

                    anchors {
                        left: parent.left
                        right: parent.right
                        bottom: parent.top
                    }
                    height: 6
                    color: colors.background
                    visible: progress > 0

                    Rectangle {
                        id: progressFill
                        anchors {
                            left: parent.left
                            top: parent.top
                            bottom: parent.bottom
                        }
                        width: parent.width * progressBar.progress
                        gradient: Gradient {
                            orientation: Gradient.Horizontal
                            GradientStop {
                                position: 1.0
                                color: progressBar.completed ? colors.midnightBlue : colors.superGreen
                            }
                            GradientStop {
                                position: progressBar.completed ? 0.5 : 0.8
                                color: progressBar.completed ? colors.periwinkle : colors.green
                            }
                            GradientStop {
                                position: 0.0
                                color: progressBar.completed ? colors.midnightBlue : colors.green
                            }
                        }
                    }
                }
                
                Item {
                    id: titleContent
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
                            //radius: 0

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

        // Info button — direct child of card, declared LAST so it's painted
        // on top. Shown only when the card is hovered.
        Rectangle {
            id: infoButton
            anchors.top: parent.top
            anchors.right: parent.right
            anchors.margins: 10
            width: 36
            height: 36
            z: 10
            visible: cardHover.hovered || infoHover.hovered
            color: infoHover.hovered ? colors.superGreen : "#CC000000"
            border.color: colors.superGreen
            border.width: 2

            Behavior on color { ColorAnimation { duration: 100 } }

            Text {
                anchors.centerIn: parent
                text: "i"
                color: infoHover.hovered ? "black" : colors.superGreen
                font.italic: true
                font.bold: true
                font.pointSize: 16
                font.family: "Georgia"
            }

            HoverHandler {
                id: infoHover
                cursorShape: Qt.PointingHandCursor
            }

            TapHandler {
                onTapped: card._showDetails()
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