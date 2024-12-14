#include "Navigator.h"
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
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
    m_sidebarCategories({
        {"continueWatching", "Continuar viendo"},
        {"series", "Series"},
        {"movies", "Pelis"}
    }) {

}

// Getters
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

// Setters
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

QString Navigator::getMediaTitle(QString mediaId) const {
    for (const auto& media : m_mediaData) {
        if (media["id"] == mediaId) {
            return media["title"].toString();
        }
    }
    return QString();
}




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

QVariantList Navigator::getUniqueGenres() {
    QSet<QString> uniqueGenres;

    // Extract genres from collections
    for (const QVariant& collection : m_collectionsData) {
        QVariantMap collectionMap = collection.toMap();
        QString genresJson = collectionMap["genres"].toString();
        uniqueGenres.unite(parseJson(genresJson));
    }

    // Extract genres from individual media items (if they have genres)
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

QStringList Navigator::getUniqueProducers() {
    QSet<QString> producers;
    //producers.insert("All"); // Always include "All" option

    // Extract producers from collections
    for (const QVariant& collection : m_collectionsData) {
        QVariantMap collectionMap = collection.toMap();
        QString producer = collectionMap["producer"].toString();
        if (!producer.isEmpty()) {
            producers.insert(producer);
        }
    }

    // Extract producers from media items
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

void Navigator::clearFilters() {
    //m_selectedDirector = "";
    m_selectedProducer = "";
    m_selectedGenres.clear();
    m_selectedEras.clear();
    m_showTopRated = false;
    
    //emit selectedDirectorChanged();
    emit selectedProducerChanged();
    emit selectedGenresChanged();
    emit selectedErasChanged();
    emit showTopRatedChanged();
}

QString Navigator::getNextEpisode(QString currentMediaId, int index) {
    int current_episode = -1;
    for (const auto& media : m_mediaData) {
        if (media["ID"] == currentMediaId) {
            current_episode = media["episode"].toInt();
        }
    }
    for (const auto& media : m_mediaData) {
        if (media["collection_id"].toString() == m_selectedCollectionId && media["ID"] == currentMediaId) {
            if (media["episode"].toInt() == current_episode + index) {
                return media["ID"].toString();
            }
        }
    }
    return QString();

}

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

bool Navigator::anyValueContains(const QVariantMap& map, const QString& substring) const {
    for (auto it = map.cbegin(); it != map.cend(); ++it) {
        if (it.value().metaType().id() == QMetaType::QString) { // Check if the value is a QString
            if (it.value().toString().contains(substring, Qt::CaseInsensitive)) {
                return true;
            }
        }
    }
    return false;
}

QVariantMap Navigator::getCollection(QString collectionId) {
    for (const auto& collection : m_collectionsData) {
        if (collection["ID"] == collectionId) {
            return collection;
        }
    }
    return QVariantMap();
}

QString Navigator::getEraFromYear(int year) {
    if (year <= 0) return QString();
    int decade = (year / 10) * 10;
    return QString::number(decade) + "'s";
}

void Navigator::updateFilteredData() {
    updateFilteredData(""); // Call the version with the default value
}

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

void Navigator::filterByChosenCollection(QList<QVariantMap>& filteredMedia, QList<QVariantMap>& filteredCollections){
    if (m_selectedCollectionId != "") {
        filteredCollections.clear();
        QList<QVariantMap> _tmp;
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


void Navigator::updateFilteredData(QString searchText) {
    QList<QVariantMap> filteredMedia;
    QList<QVariantMap> filteredCollections;
    
    filterByCategory(filteredMedia, filteredCollections); // First always, fills variables
    filterByChosenCollection(filteredMedia, filteredCollections);
    filterBySearchText(filteredMedia, filteredCollections, searchText);

    // Genres - collections
    // Rate - collections and medias
    // Producer - collections and medias
    // Eras - medias


    filterByFilterBarMedia(filteredMedia);
    filterByFilterBarCollection(filteredCollections);
    
    if (m_currentCategory == "continueWatching" or m_selectedCollectionId != "") {
        filteredCollections.clear();
    }
    else {
        if (!m_groupByCollection and m_currentCategory == "movies") {
            filteredCollections.clear();
        }
        else {
            for (const auto& collection : filteredCollections) {
                for (const auto& media : getMediaByCollection(collection["ID"].toString())) {
                    filteredMedia.removeAll(media);
                }
            }
        }
    }

    m_filteredData = filteredMedia + filteredCollections;
    emit filteredDataChanged();
}
void Navigator::filterByFilterBarCollection(QList<QVariantMap>& filteredCollections) {
    QList<QString> filteredCollectionIDs;
    for (const auto& collection : filteredCollections) {
        filteredCollectionIDs.append(collection["ID"].toString());
    }

    for (const auto& collection : filteredCollections) {
        bool remove = false;
        // If media collection has any genre, use it
        if (m_selectedGenres.size() > 0) {
            bool isGenre = false;
            for (const auto& genre : m_selectedGenres) {
                for (const auto& mediaGenre : parseGenres(collection["genres"].toString())) {
                    isGenre = isGenre || genre == mediaGenre;
                }            
            }
            remove = remove || !isGenre;
        }

        if (m_selectedEras.size() > 0) {
            bool isEra = false;
            for (const auto& era : m_selectedEras) {
                for (const auto& media : getMediaByCollection(collection["ID"].toString())) {
                    isEra = isEra || era == getEraFromYear(collection["year"].toInt());
                }
            }
            remove = remove || !isEra;
        }
        
        bool isProducer = m_selectedProducer == "" or m_selectedProducer == collection["producer"];
        for (const auto& media : getMediaByCollection(collection["ID"].toString())) {
            isProducer = isProducer || m_selectedProducer == media["producer"];
        }
        remove = remove || !isProducer;

        if (m_showTopRated) {
            double rating = collection["collection_rating"].toDouble();
            remove = remove || rating < 8.0;
        }

        if (!remove) {
            filteredCollectionIDs.removeAll(collection["ID"].toString());
        }
    }

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
void Navigator::filterByFilterBarMedia(QList<QVariantMap>& filteredMedia) {
    QList<QString> filteredMediaIDs;
    for (const auto& media : filteredMedia) {
        filteredMediaIDs.append(media["ID"].toString());
    }

    for (const auto& media : filteredMedia) {
        bool remove = false;
        // If media collection has any genre, use it
        if (m_selectedGenres.size() > 0) {
            bool isGenre = false;
            for (const auto& genre : m_selectedGenres) {
                if (media["collection_id"].toString() != "") {
                    QVariantMap collection_genres = getCollection(media["collection_id"].toString());
                    for (const auto& mediaGenre : parseGenres(collection_genres["genres"].toString())) {
                        isGenre = isGenre || genre == mediaGenre;
                    }
                }
                if (media["genres"] != "") {
                    for (const auto& mediaGenre : parseGenres(media["genres"].toString())) {
                        isGenre = isGenre || genre == mediaGenre;
                    }
                }
                

            }
            remove = remove || !isGenre;
        }

        if (m_selectedEras.size() > 0) {
            bool isEra = false;
            for (const auto& era : m_selectedEras) {
                isEra = isEra || era == getEraFromYear(media["year"].toInt());
            }
            remove = remove || !isEra;
        }

        remove = remove || m_selectedProducer != "" and m_selectedProducer != media["producer"];

        if (m_showTopRated) {
            remove = remove || media["rating"].toDouble() < 8.0;
        }

        if (!remove) {
            filteredMediaIDs.removeAll(media["ID"].toString());
        }
    }

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

void Navigator::filterBySearchText(QList<QVariantMap>& filteredMedia, QList<QVariantMap>& filteredCollections, QString searchText) {   
    if (searchText != "") {
        QList<QVariantMap> _tmp;
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

void Navigator::filterByCategory(QList<QVariantMap>& filteredMedia, QList<QVariantMap>& filteredCollections) {
    for (const auto& media : m_mediaData) {

        if (m_currentCategory == "continueWatching") {
            if (getMediaProgress(media["ID"].toString()) > 0) {
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
    if (m_currentCategory != "continueWatching") {
        for (const auto& collection : m_collectionsData) {
            QString type = collection["collection_type"].toString();
            if (m_currentCategory == "series" and type == "serie" or m_currentCategory == "movies" and type == "movies") {
                filteredCollections.append(collection);
            }
        }
    }
}



bool Navigator::mediaInCollection(QString mediaId, QString collectionId) {
    for (const auto& media : m_mediaData) {
        if (media["collection_id"].toString() == collectionId) {
            return true;
        }
    }
    return false;
}

QList<QVariantMap> Navigator::getMediaByCollection(QString collectionId) {
    QList<QVariantMap> result;
    for (const auto& media : m_mediaData) {
        if (media["collection_id"] == collectionId) {
            result.append(media);
        }
    }
    return result;
}

void Navigator::setMediaData(QList<QVariantMap> mediaData) {
    m_mediaData = mediaData;
    emit mediaDataChanged();
    
    
}

void Navigator::setCollectionsData(QList<QVariantMap> collectionsData) {
    m_collectionsData = collectionsData;
    emit collectionsDataChanged();
}

void Navigator::setMediaMetadata(QList<QVariantMap> mediaMetadata) {
    for (const auto& media : mediaMetadata) {
        m_mediaMetadata[media["mediaID"].toString()] = media;
    }
    emit mediaMetadataChanged();
    emit mediaLoaded();
    updateFilteredData();
}
