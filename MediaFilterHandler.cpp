#include "MediaFilterHandler.h"
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QDebug>

MediaFilterHandler::MediaFilterHandler(QObject* parent) : QObject(parent) {}

QSet<QString> MediaFilterHandler::parseGenres(const QString& genresJson) {
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

QVariantList MediaFilterHandler::getUniqueGenres(const QVariantList& collections, const QVariantList& media) {
    QSet<QString> uniqueGenres;

    // Extract genres from collections
    for (const QVariant& collection : collections) {
        QVariantMap collectionMap = collection.toMap();
        QString genresJson = collectionMap["genres"].toString();
        uniqueGenres.unite(parseGenres(genresJson));
    }

    // Extract genres from individual media items (if they have genres)
    for (const QVariant& mediaItem : media) {
        QVariantMap mediaMap = mediaItem.toMap();
        QString genresJson = mediaMap["genres"].toString();
        uniqueGenres.unite(parseGenres(genresJson));
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

QStringList MediaFilterHandler::getUniqueProducers(const QVariantList& collections, const QVariantList& media) {
    QSet<QString> producers;
    //producers.insert("All"); // Always include "All" option

    // Extract producers from collections
    for (const QVariant& collection : collections) {
        QVariantMap collectionMap = collection.toMap();
        QString producer = collectionMap["producer"].toString();
        if (!producer.isEmpty()) {
            producers.insert(producer);
        }
    }

    // Extract producers from media items
    for (const QVariant& mediaItem : media) {
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

bool MediaFilterHandler::mediaMatchesGenres(const QVariantMap& media, const QStringList& genres) {
    if (genres.isEmpty()) return true;

    QSet<QString> mediaGenres = parseGenres(media["genres"].toString());
    for (const QString& genre : genres) {
        if (mediaGenres.contains(genre)) {
            return true;
        }
    }
    return false;
}

double MediaFilterHandler::parseRating(const QVariant& rating) {
    bool ok;
    double value = rating.toDouble(&ok);
    return ok ? value : 0.0;
}

QVariantList MediaFilterHandler::filterByGenre(const QVariantList& data, const QStringList& genres) {
    if (genres.isEmpty()) return data;

    QVariantList filtered;
    for (const QVariant& item : data) {
        QVariantMap itemMap = item.toMap();
        if (mediaMatchesGenres(itemMap, genres)) {
            filtered.append(item);
        }
    }
    return filtered;
}

QVariantList MediaFilterHandler::filterByRating(const QVariantList& data, double minRating) {
    QVariantList filtered;
    for (const QVariant& item : data) {
        QVariantMap itemMap = item.toMap();
        double rating = parseRating(itemMap["rating"]) != 0 ?
            parseRating(itemMap["rating"]) :
            parseRating(itemMap["collection_rating"]);
        if (rating >= minRating) {
            filtered.append(item);
        }
        
    }
    return filtered;
}

QString MediaFilterHandler::getEraFromYear(int year) {
    if (year <= 0) return QString();
    int decade = (year / 10) * 10;
    return QString::number(decade) + "'s";
}

QVariantList MediaFilterHandler::filterByEra(const QVariantList& data, const QStringList& eras) {
    if (eras.isEmpty()) return data;

    QVariantList filtered;
    for (const QVariant& item : data) {
        QVariantMap itemMap = item.toMap();
        QString era = getEraFromYear(itemMap["year"].toInt());
        if (eras.contains(era)) {
            filtered.append(item);
        }
    }
    return filtered;
}

QVariantList MediaFilterHandler::filterByProducer(const QVariantList& data, const QString& producer) {
    if (producer.isEmpty() || producer == "All") return data;

    QVariantList filtered;
    for (const QVariant& item : data) {
        QVariantMap itemMap = item.toMap();
        if (itemMap["producer"].toString() == producer) {
            filtered.append(item);
        }
    }
    return filtered;
}