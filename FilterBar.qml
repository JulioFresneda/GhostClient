// FilterBar.qml
Rectangle {
    id: filterBar
    Layout.fillWidth: true
    height: 56
    color: colors.surface
    radius: 8

    // Properties to store selected filters
    property var selectedGenres: []
    property string selectedEra: ""
    property bool showTopRated: false
    property string selectedDirector: ""
    property string selectedProducer: ""

    // Signals for filter changes
    signal filtersChanged()

    RowLayout {
        anchors {
            fill: parent
            margins: 8
        }
        spacing: 16

        // Genres ComboBox
        ComboBox {
            id: genreFilter
            Layout.preferredWidth: 150
            model: getUniqueGenres()
            textRole: "text"
            displayText: selectedGenres.length > 0 ? 
                        selectedGenres.join(", ") : "Genre"

            delegate: CheckDelegate {
                width: parent.width
                text: modelData.text
                checked: selectedGenres.includes(modelData.text)
                onCheckedChanged: {
                    if (checked) {
                        if (!selectedGenres.includes(modelData.text)) {
                            selectedGenres.push(modelData.text)
                        }
                    } else {
                        const index = selectedGenres.indexOf(modelData.text)
                        if (index > -1) {
                            selectedGenres.splice(index, 1)
                        }
                    }
                    filterBar.filtersChanged()
                }
            }
        }

        // Era ComboBox
        ComboBox {
            id: eraFilter
            Layout.preferredWidth: 120
            model: ["All", "60's", "70's", "80's", "90's", "2000's", "2010's", "2020's"]
            displayText: currentText === "All" ? "Era" : currentText
            onActivated: {
                selectedEra = currentText === "All" ? "" : currentText
                filterBar.filtersChanged()
            }
        }

        // Top Rated Switch
        Switch {
            id: topRatedSwitch
            text: "Top Rated (8+)"
            checked: showTopRated
            onCheckedChanged: {
                showTopRated = checked
                filterBar.filtersChanged()
            }
        }

        // Producer ComboBox
        ComboBox {
            id: producerFilter
            Layout.preferredWidth: 150
            model: getUniqueProducers()
            displayText: currentText || "Producer"
            onActivated: {
                selectedProducer = currentText === "All" ? "" : currentText
                filterBar.filtersChanged()
            }
        }

        // Clear Filters Button
        Button {
            text: "Clear Filters"
            flat: true
            visible: hasActiveFilters()
            onClicked: clearFilters()

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

    // Helper function to get unique genres from collections and media
    function getUniqueGenres() {
        const genres = new Set()
        collectionsData.forEach(collection => {
            try {
                const genreList = JSON.parse(collection.genres || "[]")
                genreList.forEach(genre => genres.add(genre))
            } catch (e) {
                console.error("Error parsing genres:", e)
            }
        })
        return Array.from(genres).map(genre => ({ text: genre }))
    }

    // Helper function to get unique producers
    function getUniqueProducers() {
        const producers = new Set(["All"])
        collectionsData.forEach(collection => {
            if (collection.producer) {
                producers.add(collection.producer)
            }
        })
        mediaData.forEach(media => {
            if (media.producer) {
                producers.add(media.producer)
            }
        })
        return Array.from(producers)
    }

    // Helper function to check if any filters are active
    function hasActiveFilters() {
        return selectedGenres.length > 0 || 
               selectedEra !== "" || 
               showTopRated || 
               selectedProducer !== ""
    }

    // Helper function to clear all filters
    function clearFilters() {
        selectedGenres = []
        selectedEra = ""
        showTopRated = false
        selectedProducer = ""
        genreFilter.currentIndex = -1
        eraFilter.currentIndex = 0
        topRatedSwitch.checked = false
        producerFilter.currentIndex = 0
        filterBar.filtersChanged()
    }

    // Helper function to get era from year
    function getEraFromYear(year) {
        if (!year) return ""
        const decade = Math.floor(year / 10) * 10
        return decade + "'s"
    }
}