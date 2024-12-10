#ifndef MEDIAFILTERHANDLER_H
#define MEDIAFILTERHANDLER_H

#include <QObject>
#include <QVariantList>
#include <QVariantMap>
#include <QString>
#include <QSet>

class MediaFilterHandler : public QObject {
    Q_OBJECT

public:
    explicit MediaFilterHandler(QObject* parent = nullptr);

    Q_INVOKABLE QVariantList getUniqueGenres(const QVariantList& collections, const QVariantList& media);
    Q_INVOKABLE QStringList getUniqueProducers(const QVariantList& collections, const QVariantList& media);
    Q_INVOKABLE QVariantList filterByGenre(const QVariantList& data, const QStringList& genres);
    Q_INVOKABLE QVariantList filterByRating(const QVariantList& data, double minRating);
    Q_INVOKABLE QVariantList filterByEra(const QVariantList& data, const QStringList& eras);
    Q_INVOKABLE QVariantList filterByProducer(const QVariantList& data, const QString& producer);
    Q_INVOKABLE QString getEraFromYear(int year);

private:
    QSet<QString> parseGenres(const QString& genresJson);
    bool mediaMatchesGenres(const QVariantMap& media, const QStringList& genres);
    double parseRating(const QVariant& rating);
};

#endif // MEDIAFILTERHANDLER_H