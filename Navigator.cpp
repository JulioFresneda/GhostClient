#include "Navigator.h"
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
Navigator::Navigator(QObject* parent)
    : QObject(parent),
    m_currentCategory(""),
    m_selectedCollectionId(""),
    m_showTopRated(false) {
    // Initialize your sidebar categories or other member variables here if needed.
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
        emit currentCategoryChanged();
    }
}

void Navigator::setSelectedCollectionId(const QString& collectionId) {
    if (m_selectedCollectionId != collectionId) {
        m_selectedCollectionId = collectionId;
        emit selectedCollectionIdChanged();
    }
}

void Navigator::setSelectedGenres(const QList<QString>& genres) {
    if (m_selectedGenres != genres) {
        m_selectedGenres = genres;
        emit selectedGenresChanged();
    }
}

void Navigator::setSelectedEras(const QList<QString>& eras) {
    if (m_selectedEras != eras) {
        m_selectedEras = eras;
        emit selectedErasChanged();
    }
}

void Navigator::setShowTopRated(bool showTopRated) {
    if (m_showTopRated != showTopRated) {
        m_showTopRated = showTopRated;
        emit showTopRatedChanged();
    }
}

void Navigator::setSelectedDirector(const QString& director) {
    if (m_selectedDirector != director) {
        m_selectedDirector = director;
        emit selectedDirectorChanged();
    }
}

void Navigator::setSelectedProducer(const QString& producer) {
    if (m_selectedProducer != producer) {
        m_selectedProducer = producer;
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


// Private Helper
void Navigator::filterData() {
    m_filteredData.clear();
    for (const auto& item : m_mediaData) {
        if (item.contains("category") && item["category"].toString() == m_currentCategory) {
            m_filteredData.append(item);
        }
    }
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
    m_selectedDirector = "All";
    m_selectedProducer = "All";
    m_selectedGenres.clear();
    m_selectedEras.clear();
    m_showTopRated = false;
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

    QVariantMap meta = m_mediaMetadata[mediaId];

    if (meta != NULL) {
        float percentage = meta["percentageWatched"].toFloat();
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

void Navigator::updateFilteredData(QString searchText) {
    QList<QVariantMap> filteredResults;
    for (const auto& media : m_mediaData) {
        if (media["collection_id"] == m_selectedCollectionId || m_selectedCollectionId == "") {
            QString collection_type = getCollection(m_selectedCollectionId)["collection_type"].toString();
            if (m_currentCategory == "series" and collection_type == "serie" or m_currentCategory == "movies" and collection_type == "movies") {
                if (anyValueContains(media, searchText)) {
                    filteredResults.append(media);
                }
            }
            if (m_currentCategory == "continueWatching" and getMediaProgress(media["ID"].toString())) {
                if (anyValueContains(media, searchText)) {
                    filteredResults.append(media);
                }
            }
            
        }
    }

    QList<QString> filteredBarIDs;
    for (const auto& media : filteredResults) {
        filteredBarIDs.append(media["ID"].toString());
    }

    for (const auto& media : filteredResults) {
        bool remove = false;
        if (m_selectedGenres.size() > 0) {
            bool isGenre = false;
            for (const auto& genre : m_selectedGenres) {
                isGenre = isGenre || genre == media["genre"];
            }
            remove = remove || !isGenre;
        }

        if (m_selectedEras.size() > 0) {
            bool isEra = false;
            for (const auto& era : m_selectedEras) {
                isEra = isEra || era == getEraFromYear(media["era"].toInt());
            }
            remove = remove || !isEra;
        }

        remove = remove || m_selectedProducer != "" and m_selectedProducer != media["selectedProducer"];

        if (m_showTopRated) {
            remove = remove || media["rating"].toInt() < 8;
        }

        if (remove) {
            filteredBarIDs.removeAll(media["ID"].toString());
        }
    }

    for (const auto& mediaId : filteredBarIDs) {
        filteredResults.removeAll(mediaId);
    }

    m_filteredData = filteredResults;
    emit filteredDataChanged();
}

