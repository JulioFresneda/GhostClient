/**
 * @file Navigator.cpp
 * @brief Implementation of the Navigator class which manages media content filtering, sorting and navigation
 * for a media streaming application
 */

#include "Navigator.h"
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>

 /**
  * @brief Constructor for Navigator class
  * @param parent Parent QObject
  * Initializes the navigator with default values and category mappings
  */
Navigator::Navigator(QObject* parent)
    : QObject(parent),
    m_currentCategory("continueWatching"),
    m_selectedCollectionId(""),
    m_selectedGenres(),
    m_selectedEras(),
    m_showTopRated(false),
    m_groupByCollection(true),
    m_selectedDirector(""),
    m_selectedProducer(""),
    m_filteredData(),
    m_sortBy("title"),
    m_sortOrder("asc"),
    m_sidebarCategories({
        {"continueWatching", "Continue Watching"},
        {"series", "Series"},
        {"movies", "Movies"}
        }) {

}

// Getter methods for class properties
QList<QVariantMap> Navigator::collectionsData() const {
    return m_collectionsData;
}

QList<QVariantMap> Navigator::mediaData() const {
    return m_mediaData;
}

QList<QVariantMap> Navigator::filteredData() const {
    return m_filteredData;
}

QMap<QString, QVariantMap> Navigator::mediaMetadata() const {
    return m_mediaMetadata;
}

QString Navigator::currentCategory() const {
    return m_currentCategory;
}

QString Navigator::selectedCollectionId() const {
    return m_selectedCollectionId;
}

QList<QString> Navigator::selectedGenres() const {
    return m_selectedGenres;
}

QList<QString> Navigator::selectedEras() const {
    return m_selectedEras;
}

bool Navigator::showTopRated() const {
    return m_showTopRated;
}

bool Navigator::groupByCollection() const {
    return m_groupByCollection;
}

QString Navigator::selectedDirector() const {
    return m_selectedDirector;
}

QString Navigator::selectedProducer() const {
    return m_selectedProducer;
}

QString Navigator::sortBy() const {
    return m_sortBy;
}

bool Navigator::sortOrder() const {
    return m_sortOrder;
}

/**
 * @brief Setter methods for class properties
 * Each setter updates the corresponding property and triggers updateFilteredData()
 * to refresh the displayed content
 */
void Navigator::setCurrentCategory(const QString& category) {
    if (m_currentCategory != category) {
        m_currentCategory = category;
        updateFilteredData();
        emit currentCategoryChanged();

    }
}

void Navigator::setSelectedCollectionId(const QString& collectionId) {
    if (m_selectedCollectionId != collectionId) {
        m_selectedCollectionId = collectionId;
        updateFilteredData();
        emit selectedCollectionIdChanged();
    }
}

void Navigator::setSelectedGenres(const QList<QString>& genres) {
    if (m_selectedGenres != genres) {
        m_selectedGenres = genres;
        updateFilteredData();
        emit selectedGenresChanged();
    }
}

void Navigator::setSelectedEras(const QList<QString>& eras) {
    if (m_selectedEras != eras) {
        m_selectedEras = eras;
        updateFilteredData();
        emit selectedErasChanged();
    }
}

void Navigator::setShowTopRated(bool showTopRated) {
    if (m_showTopRated != showTopRated) {
        m_showTopRated = showTopRated;
        updateFilteredData();
        emit showTopRatedChanged();
    }
}

void Navigator::setGroupByCollection(bool groupByCollection) {
    if (m_groupByCollection != groupByCollection) {
        m_groupByCollection = groupByCollection;
        updateFilteredData();
        emit groupByCollectionChanged();
    }
}

void Navigator::setSelectedDirector(const QString& director) {
    if (m_selectedDirector != director) {
        m_selectedDirector = director;
        updateFilteredData();
        emit selectedDirectorChanged();
    }
}

void Navigator::setSelectedProducer(const QString& producer) {
    if (m_selectedProducer != producer) {
        m_selectedProducer = producer;
        updateFilteredData();
        emit selectedProducerChanged();
    }
}

void Navigator::setSortBy(const QString& sortBy) {
    m_sortBy = sortBy.toLower();
    updateFilteredData();
    emit sortByChanged();
}

void Navigator::setSortOrder(const bool sortOrder) {
    m_sortOrder = sortOrder;
    updateFilteredData();
    emit sortOrderChanged();
}

/**
 * @brief Returns sidebar categories as a list of key-value pairs
 * @return QVariantList containing category information
 */
QVariantList Navigator::sidebarCategories() const {
    QVariantList categories;
    for (auto it = m_sidebarCategories.cbegin(); it != m_sidebarCategories.cend(); ++it) {
        QVariantMap category;
        category["key"] = it.key();
        category["value"] = it.value();
        categories.append(category);
    }
    return categories;
}

/**
 * @brief Gets the previous category in the sidebar navigation
 * @return QString containing the previous category key
 */
QString Navigator::getPreviousCategory() const {
    bool previous = true;
    QString value = "";

    // Iterate through categories until current is found
    for (auto it = m_sidebarCategories.cbegin(); it != m_sidebarCategories.cend(); ++it) {

        if (it.key() == m_currentCategory) {
            previous = false;
        }
        if (previous) {
            value = it.key();
        }

    }
    return value;
}

/**
 * @brief Gets the next category in the sidebar navigation
 * @return QString containing the next category key
 */
QString Navigator::getNextCategory() const {
    bool next = false;
    for (auto it = m_sidebarCategories.cbegin(); it != m_sidebarCategories.cend(); ++it) {
        if (it.key() == m_currentCategory) {
            next = true;
        }
        else if (next) {
            return it.key();
        }
    }
    return "";
}

/**
 * @brief Gets the formatted title for a media item
 * @param mediaId ID of the media item
 * @return Formatted title string including season/episode for TV shows
 */
QString Navigator::getMediaTitle(QString mediaId) const {
    for (const auto& media : m_mediaData) {
        if (media["ID"] == mediaId) {
            QString title = "";
            if (media["type"] == "episode") {
                title += "[S" + media["season"].toString() + " EP" + media["episode"].toString() + "] ";
            }
            title += media["title"].toString();
            return title;
        }
    }
    return QString();
}

/**
 * @brief Parses a JSON array string into a set of strings
 * @param json JSON string to parse
 * @return QSet<QString> containing unique strings from the JSON array
 */
QSet<QString> Navigator::parseJson(const QString& json) {
    QSet<QString> parsed;
    QJsonDocument doc = QJsonDocument::fromJson(json.toUtf8());
    if (doc.isArray()) {
        QJsonArray array = doc.array();
        for (const QJsonValue& value : array) {
            if (value.isString()) {
                parsed.insert(value.toString());
            }
        }
    }
    return parsed;
}

/**
 * @brief Gets all unique genres from collections and media items
 * @return QVariantList containing unique genres as text objects
 */
QVariantList Navigator::getUniqueGenres() {
    QSet<QString> uniqueGenres;

    // Extract genres from collections and media items
    for (const QVariant& collection : m_collectionsData) {
        QVariantMap collectionMap = collection.toMap();
        QString genresJson = collectionMap["genres"].toString();
        uniqueGenres.unite(parseJson(genresJson));
    }

    for (const QVariant& mediaItem : m_mediaData) {
        QVariantMap mediaMap = mediaItem.toMap();
        QString genresJson = mediaMap["genres"].toString();
        uniqueGenres.unite(parseJson(genresJson));
    }

    // Convert to list of maps for QML
    QVariantList result;
    for (const QString& genre : uniqueGenres) {
        QVariantMap genreMap;
        genreMap["text"] = genre;
        result.append(genreMap);
    }

    return result;
}

/**
 * @brief Gets all unique producers from collections and media items
 * @return QStringList containing unique producers with "All" as first option
 */
QStringList Navigator::getUniqueProducers() {
    QSet<QString> producers;

    // Extract producers from collections and media items
    for (const QVariant& collection : m_collectionsData) {
        QVariantMap collectionMap = collection.toMap();
        QString producer = collectionMap["producer"].toString();
        if (!producer.isEmpty()) {
            producers.insert(producer);
        }
    }

    for (const QVariant& mediaItem : m_mediaData) {
        QVariantMap mediaMap = mediaItem.toMap();
        QString producer = mediaMap["producer"].toString();
        if (!producer.isEmpty()) {
            producers.insert(producer);
        }
    }

    QStringList producers_list = producers.values();
    producers_list.prepend("All");
    return producers_list;
}

/**
 * @brief Clears all active filters except director
 */
void Navigator::clearFilters() {
    m_selectedProducer = "";
    m_selectedGenres.clear();
    m_selectedEras.clear();
    m_showTopRated = false;

    emit selectedProducerChanged();
    emit selectedGenresChanged();
    emit selectedErasChanged();
    emit showTopRatedChanged();
}

/**
 * @brief Determines the type of episode (first, middle, or final)
 * @param currentMediaId ID of the current media item
 * @return QString indicating episode type
 */
QString Navigator::getEpisodeType(QString currentMediaId) {
    QVariantMap media = getMedia(currentMediaId);
    if (media["type"] == "episode") {
        if (media["episode"].toInt() == 1) {
            return "FirstEpisode";
        }
        else if (media["ID"] == getFinalEpisode()) {
            return "FinalEpisode";
        }
        else {
            return "MiddleEpisode";
        }
    }
    return "NoEpisode";
}

/**
 * @brief Gets the ID of the final episode in the current collection
 * @return QString containing the final episode's ID
 */
QString Navigator::getFinalEpisode() {
    int max = -1;
    QString mediaId = "";
    for (const auto& media : m_mediaData) {
        if (media["collection_id"].toString() == m_selectedCollectionId) {
            if (media["episode"].toInt() > max) {
                max = media["episode"].toInt();
                mediaId = media["ID"].toString();
            }
        }
    }
    return mediaId;
}

/**
 * @brief Gets the ID of the next episode relative to the current one
 * @param currentMediaId Current episode ID
 * @param index Number of episodes to look ahead
 * @return QString containing the next episode's ID
 */
QString Navigator::getNextEpisode(QString currentMediaId, int index) {
    int current_episode = -1;
    // Find current episode number
    for (const auto& media : m_mediaData) {
        if (media["ID"] == currentMediaId) {
            current_episode = media["episode"].toInt();
        }
    }
    // Find episode with number current + index
    for (const auto& media : m_mediaData) {
        if (media["collection_id"].toString() == m_selectedCollectionId) {
            if (media["episode"].toInt() == current_episode + index) {
                return media["ID"].toString();
            }
        }
    }
    return QString();
}

/**
 * @brief Gets the watching progress for a media item
 * @param mediaId ID of the media item
 * @return float containing progress percentage (0.0 - 1.0)
 */
float Navigator::getMediaProgress(QString mediaId) {
    if (m_mediaMetadata.contains(mediaId)) {
        QVariantMap meta = m_mediaMetadata[mediaId];
        float percentage = meta["percentage_watched"].toFloat();
        if (percentage) {
            return percentage;
        }
    }
    return 0.0;
}

/**
 * @brief Checks if any value in a QVariantMap contains a substring
 * @param map Map to search
 * @param substring String to search for
 * @return bool indicating if substring was found
 */
bool Navigator::anyValueContains(const QVariantMap& map, const QString& substring) const {
    for (auto it = map.cbegin(); it != map.cend(); ++it) {
        if (it.value().metaType().id() == QMetaType::QString) {
            if (it.value().toString().contains(substring, Qt::CaseInsensitive)) {
                return true;
            }
        }
    }
    return false;
}

/**
 * @brief Gets collection information by ID
 * @param collectionId ID of the collection
 * @return QVariantMap containing collection data
 */
QVariantMap Navigator::getCollection(QString collectionId) {
    for (const auto& collection : m_collectionsData) {
        if (collection["ID"] == collectionId) {
            return collection;
        }
    }
    return QVariantMap();
}

/**
 * @brief Converts a year to its decade string (e.g., "1990's")
 * @param year Year to convert
 * @return QString containing the decade
 */
QString Navigator::getEraFromYear(int year) {
    if (year <= 0) return QString();
    int decade = (year / 10) * 10;
    return QString::number(decade) + "'s";
}

/**
 * @brief Updates filtered data with default empty search text
 */
void Navigator::updateFilteredData() {
    updateFilteredData("");
}

/**
 * @brief Parses genres JSON string into a set of unique genres
 * @param genresJson JSON string containing genres
 * @return QSet<QString> containing unique genres
 */
QSet<QString> Navigator::parseGenres(const QString& genresJson) {
    QSet<QString> genres;
    QJsonDocument doc = QJsonDocument::fromJson(genresJson.toUtf8());
    if (doc.isArray()) {
        QJsonArray array = doc.array();
        for (const QJsonValue& value : array) {
            if (value.isString()) {
                genres.insert(value.toString());
            }
        }
    }
    return genres;
}

/**
 * @brief Filters media based on selected collection
 * @param filteredMedia Reference to filtered media list
 * @param filteredCollections Reference to filtered collections list
 */
void Navigator::filterByChosenCollection(QList<QVariantMap>& filteredMedia, QList<QVariantMap>& filteredCollections) {
    if (m_selectedCollectionId != "") {
        filteredCollections.clear();
        QList<QVariantMap> _tmp;
        // Remove media not in selected collection
        for (const auto& media : filteredMedia) {
            if (media["collection_id"] != m_selectedCollectionId) {
                _tmp.append(media);
            }
        }
        for (const auto& media : _tmp) {
            filteredMedia.removeAll(media);
        }
    }
}

/**
 * @brief Gets metadata for a specific media item
 * @param mediaId ID of the media item
 * @return QVariantMap containing media metadata
 */
QVariantMap Navigator::getMediaMetadata(QString mediaId) {
    return m_mediaMetadata[mediaId];
}

/**
 * @brief Updates filtered data based on current filters and search text
 * @param searchText Text to filter results by
 */
void Navigator::updateFilteredData(QString searchText) {
    QList<QVariantMap> filteredMedia;
    QList<QVariantMap> filteredCollections;

    // Apply filters in sequence
    filterByCategory(filteredMedia, filteredCollections);
    filterByChosenCollection(filteredMedia, filteredCollections);
    filterBySearchText(filteredMedia, filteredCollections, searchText);
    filterByFilterBarMedia(filteredMedia);
    filterByFilterBarCollection(filteredCollections);

    // Handle special cases for different categories
    if (m_currentCategory == "continueWatching" or m_selectedCollectionId != "") {
        filteredCollections.clear();
    }
    else {
        if (!m_groupByCollection and m_currentCategory == "movies") {
            filteredCollections.clear();
        }
        else {
            // Remove media items that are part of displayed collections
            for (const auto& collection : filteredCollections) {
                for (const auto& media : getMediaByCollection(collection["ID"].toString())) {
                    filteredMedia.removeAll(media);
                }
            }
        }
    }

    m_filteredData = filteredMedia + filteredCollections;
    sortFilteredData();
    emit filteredDataChanged();
}

/**
 * @brief Sorts filtered data based on current sort settings
 */
void Navigator::sortFilteredData() {
    if (m_selectedCollectionId != "") {
        // Sort within collection
        if (m_currentCategory == "series") {
            sortBySeasonAndEpisode(m_filteredData);
        }
        else if (m_currentCategory == "movies") {
            sortByDouble(m_filteredData, "year", "year");
        }
        else {
            sortByDouble(m_filteredData, "rating", "collection_rating");
        }
    }
    else {
        // Sort all content
        if (m_sortBy == "title") {
            sortByString(m_filteredData, "title", "collection_title");
        }
        else if (m_sortBy == "year") {
            sortByYear(m_filteredData);
        }
        else if (m_sortBy == "rating") {
            sortByDouble(m_filteredData, "rating", "collection_rating");
        }
    }
}

/**
 * @brief Filters collections based on current filter settings
 * @param filteredCollections Reference to list of filtered collections
 */
void Navigator::filterByFilterBarCollection(QList<QVariantMap>& filteredCollections) {
    QList<QString> filteredCollectionIDs;
    // Create list of collection IDs
    for (const auto& collection : filteredCollections) {
        filteredCollectionIDs.append(collection["ID"].toString());
    }

    // Apply filters to each collection
    for (const auto& collection : filteredCollections) {
        bool remove = false;

        // Filter by genres if any are selected
        if (m_selectedGenres.size() > 0) {
            bool isGenre = false;
            for (const auto& genre : m_selectedGenres) {
                for (const auto& mediaGenre : parseGenres(collection["genres"].toString())) {
                    isGenre = isGenre || genre == mediaGenre;
                }
            }
            remove = remove || !isGenre;
        }

        // Filter by eras if any are selected
        if (m_selectedEras.size() > 0) {
            bool isEra = false;
            for (const auto& era : m_selectedEras) {
                for (const auto& media : getMediaByCollection(collection["ID"].toString())) {
                    isEra = isEra || era == getEraFromYear(collection["year"].toInt());
                }
            }
            remove = remove || !isEra;
        }

        // Filter by producer
        bool isProducer = m_selectedProducer == "" or m_selectedProducer == collection["producer"];
        for (const auto& media : getMediaByCollection(collection["ID"].toString())) {
            isProducer = isProducer || m_selectedProducer == media["producer"];
        }
        remove = remove || !isProducer;

        // Filter by rating
        if (m_showTopRated) {
            double rating = collection["collection_rating"].toDouble();
            remove = remove || rating < 8.0;
        }

        // Remove collection if it doesn't match filters
        if (!remove) {
            filteredCollectionIDs.removeAll(collection["ID"].toString());
        }
    }

    // Remove filtered collections
    for (const auto& mediaId : filteredCollectionIDs) {
        QVariantMap fm;
        for (const auto& fmedia : filteredCollections) {
            if (fmedia["ID"] == mediaId) {
                fm = fmedia;
            }
        }
        filteredCollections.removeAll(fm);
    }
}

/**
 * @brief Filters media items based on current filter settings
 * @param filteredMedia Reference to list of filtered media items
 */
void Navigator::filterByFilterBarMedia(QList<QVariantMap>& filteredMedia) {
    QList<QString> filteredMediaIDs;
    // Create list of media IDs
    for (const auto& media : filteredMedia) {
        filteredMediaIDs.append(media["ID"].toString());
    }

    // Apply filters to each media item
    for (const auto& media : filteredMedia) {
        bool remove = false;

        // Filter by genres
        if (m_selectedGenres.size() > 0) {
            bool isGenre = false;
            for (const auto& genre : m_selectedGenres) {
                // Check collection genres
                if (media["collection_id"].toString() != "") {
                    QVariantMap collection_genres = getCollection(media["collection_id"].toString());
                    for (const auto& mediaGenre : parseGenres(collection_genres["genres"].toString())) {
                        isGenre = isGenre || genre == mediaGenre;
                    }
                }
                // Check individual media genres
                if (media["genres"] != "") {
                    for (const auto& mediaGenre : parseGenres(media["genres"].toString())) {
                        isGenre = isGenre || genre == mediaGenre;
                    }
                }
            }
            remove = remove || !isGenre;
        }

        // Filter by eras
        if (m_selectedEras.size() > 0) {
            bool isEra = false;
            for (const auto& era : m_selectedEras) {
                isEra = isEra || era == getEraFromYear(media["year"].toInt());
            }
            remove = remove || !isEra;
        }

        // Filter by producer
        remove = remove || m_selectedProducer != "" and m_selectedProducer != media["producer"];

        // Filter by rating
        if (m_showTopRated) {
            remove = remove || media["rating"].toDouble() < 8.0;
        }

        // Keep media if it matches filters
        if (!remove) {
            filteredMediaIDs.removeAll(media["ID"].toString());
        }
    }

    // Remove filtered media
    for (const auto& mediaId : filteredMediaIDs) {
        QVariantMap fm;
        for (const auto& fmedia : filteredMedia) {
            if (fmedia["ID"] == mediaId) {
                fm = fmedia;
            }
        }
        filteredMedia.removeAll(fm);
    }
}

/**
 * @brief Filters media and collections based on search text
 * @param filteredMedia Reference to filtered media list
 * @param filteredCollections Reference to filtered collections list
 * @param searchText Text to filter by
 */
void Navigator::filterBySearchText(QList<QVariantMap>& filteredMedia, QList<QVariantMap>& filteredCollections, QString searchText) {
    if (searchText != "") {
        QList<QVariantMap> _tmp;
        // Filter media items
        for (const auto& media : filteredMedia) {
            QVariantMap collection = getCollection(media["collection_id"].toString());
            QString collection_title = collection["collection_title"].toString();

            QVariantMap _tmp_media = media;
            _tmp_media["collection_title"] = collection_title;
            if (!anyValueContains(_tmp_media, searchText)) {
                _tmp.append(media);
            }
        }
        for (const auto& media : _tmp) {
            filteredMedia.removeAll(media);
        }

        // Filter collections
        _tmp.clear();
        for (const auto& media : filteredCollections) {
            if (!anyValueContains(media, searchText)) {
                _tmp.append(media);
            }
        }
        for (const auto& media : _tmp) {
            filteredCollections.removeAll(media);
        }
        _tmp.clear();
    }
}

/**
 * @brief Filters media and collections based on current category
 * @param filteredMedia Reference to filtered media list
 * @param filteredCollections Reference to filtered collections list
 */
void Navigator::filterByCategory(QList<QVariantMap>& filteredMedia, QList<QVariantMap>& filteredCollections) {
    // Filter media items based on category
    for (const auto& media : m_mediaData) {
        if (m_currentCategory == "continueWatching") {
            if (getMediaProgress(media["ID"].toString()) > 0 and getMediaProgress(media["ID"].toString()) <= 0.99) {
                filteredMedia.append(media);
            }
        }
        if (m_currentCategory == "movies" and media["type"].toString() == "movie") {
            filteredMedia.append(media);
        }
        if (m_currentCategory == "series" and media["type"].toString() == "episode") {
            filteredMedia.append(media);
        }
    }

    // Filter collections based on category
    if (m_currentCategory != "continueWatching") {
        for (const auto& collection : m_collectionsData) {
            QString type = collection["collection_type"].toString();
            if (m_currentCategory == "series" and type == "serie" or m_currentCategory == "movies" and type == "movies") {
                filteredCollections.append(collection);
            }
        }
    }
}

/**
 * @brief Checks if a media item belongs to a collection
 * @param mediaId ID of the media item
 * @param collectionId ID of the collection
 * @return bool indicating if media belongs to collection
 */
bool Navigator::mediaInCollection(QString mediaId, QString collectionId) {
    for (const auto& media : m_mediaData) {
        if (media["collection_id"].toString() == collectionId) {
            return true;
        }
    }
    return false;
}

/**
 * @brief Gets all media items belonging to a collection
 * @param collectionId ID of the collection
 * @return QList<QVariantMap> containing media items
 */
QList<QVariantMap> Navigator::getMediaByCollection(QString collectionId) {
    QList<QVariantMap> result;
    for (const auto& media : m_mediaData) {
        if (media["collection_id"] == collectionId) {
            result.append(media);
        }
    }
    return result;
}

/**
 * @brief Sets the media data and triggers update
 * @param mediaData List of media items
 */
void Navigator::setMediaData(QList<QVariantMap> mediaData) {
    m_mediaData = mediaData;
    emit mediaDataChanged();
}

/**
 * @brief Sets the collections data and triggers update
 * @param collectionsData List of collections
 */
void Navigator::setCollectionsData(QList<QVariantMap> collectionsData) {
    m_collectionsData = collectionsData;
    emit collectionsDataChanged();
}

/**
 * @brief Sets the media metadata and triggers updates
 * @param mediaMetadata List of media metadata
 */
void Navigator::setMediaMetadata(QList<QVariantMap> mediaMetadata) {
    for (const auto& media : mediaMetadata) {
        m_mediaMetadata[media["mediaID"].toString()] = media;
    }
    emit mediaMetadataChanged();
    emit mediaLoaded();
    updateFilteredData();
}

/**
 * @brief Sorts list by string values
 * @param list Reference to list to sort
 * @param sortby_media Key for media items
 * @param sortby_collection Key for collections
 */
void Navigator::sortByString(QList<QVariantMap>& list, QString sortby_media, QString sortby_collection) {
    std::sort(list.begin(), list.end(), [&sortby_media, &sortby_collection, this](const QVariantMap& a, const QVariantMap& b) {
        QString valueA = a.contains(sortby_media) ? a[sortby_media].toString() : a.value(sortby_collection).toString();
        QString valueB = b.contains(sortby_media) ? b[sortby_media].toString() : b.value(sortby_collection).toString();
        if (this->m_sortOrder) {
            return valueA < valueB;
        }
        else {
            return valueA > valueB;
        }
        });
}

/**
 * @brief Sorts list by double values
 * @param list Reference to list to sort
 * @param sortby_media Key for media items
 * @param sortby_collection Key for collections
 */
void Navigator::sortByDouble(QList<QVariantMap>& list, QString sortby_media, QString sortby_collection) {
    std::sort(list.begin(), list.end(), [&sortby_media, &sortby_collection, this](const QVariantMap& a, const QVariantMap& b) {
        double valueA = a.contains(sortby_media) ? a[sortby_media].toDouble() : a.value(sortby_collection).toDouble();
        double valueB = b.contains(sortby_media) ? b[sortby_media].toDouble() : b.value(sortby_collection).toDouble();
        if (this->m_sortOrder) {
            return valueA < valueB;
        }
        else {
            return valueA > valueB;
        }
        });
}

/**
 * @brief Sorts list by year values
 * @param list Reference to list to sort
 */
void Navigator::sortByYear(QList<QVariantMap>& list) {
    std::sort(list.begin(), list.end(), [this](const QVariantMap& a, const QVariantMap& b) {
        int valueA = a.contains("year") ? a["year"].toInt() : this->getYearOfCollection(a["ID"].toString());
        int valueB = b.contains("year") ? b["year"].toInt() : this->getYearOfCollection(b["ID"].toString());
        if (this->m_sortOrder) {
            return valueA < valueB;
        }
        else {
            return valueA > valueB;
        }
        });
}

void Navigator::sortBySeasonAndEpisode(QList<QVariantMap>& list) {
    std::sort(list.begin(), list.end(), [](const QVariantMap& a, const QVariantMap& b) {
        // Extract season and episode values, with fallback to 0 if missing
        int seasonA = a.value("season", 0).toInt();
        int seasonB = b.value("season", 0).toInt();

        if (seasonA != seasonB) {
            return seasonA < seasonB; // Sort by season first
        }

        int episodeA = a.value("episode", 0).toInt();
        int episodeB = b.value("episode", 0).toInt();
        return episodeA < episodeB; // If seasons are equal, sort by episode
        });
}

int Navigator::getYearOfCollection(QString collectionId) {
    QList<QVariantMap> all_medias = getMediaByCollection(collectionId);
    QVariantMap collection = getCollection(collectionId);
    if (collection["type"] == "movie") {
        sortByDouble(all_medias, "year", "year");
    }
    else {
        sortBySeasonAndEpisode(all_medias);
    }
    return all_medias[0]["year"].toInt();
}

QString Navigator::getCollectionId(QString mediaId) {
    QVariantMap media = getMedia(mediaId);
    QString collectionId = media["collection_id"].toString();
    return collectionId;

}

QVariantMap Navigator::getMedia(QString mediaId) {
    for (const auto& media : m_mediaData) {
        if (media["ID"] == mediaId) {
            return media;
        }
    }
    return QVariantMap();
}