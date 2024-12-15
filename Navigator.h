#pragma once

#include <QObject>
#include <QString>
#include <QList>
#include <QVariant>



class Navigator : public QObject {
    Q_OBJECT

        Q_PROPERTY(QList<QVariantMap> collectionsData READ collectionsData NOTIFY collectionsDataChanged)
        Q_PROPERTY(QList<QVariantMap> mediaData READ mediaData NOTIFY mediaDataChanged)
        Q_PROPERTY(QMap<QString, QVariantMap> mediaMetadata READ mediaMetadata NOTIFY mediaMetadataChanged)



        Q_PROPERTY(QList<QVariantMap> filteredData READ filteredData NOTIFY filteredDataChanged)
        Q_PROPERTY(QString currentCategory READ currentCategory WRITE setCurrentCategory NOTIFY currentCategoryChanged)
        Q_PROPERTY(QString selectedCollectionId READ selectedCollectionId WRITE setSelectedCollectionId NOTIFY selectedCollectionIdChanged)
        Q_PROPERTY(bool groupByCollection READ groupByCollection WRITE setGroupByCollection NOTIFY groupByCollectionChanged)

        Q_PROPERTY(QList<QString> selectedGenres READ selectedGenres WRITE setSelectedGenres NOTIFY selectedGenresChanged)
        Q_PROPERTY(QList<QString> selectedEras READ selectedEras WRITE setSelectedEras NOTIFY selectedErasChanged)
        Q_PROPERTY(bool showTopRated READ showTopRated WRITE setShowTopRated NOTIFY showTopRatedChanged)
        Q_PROPERTY(QString selectedDirector READ selectedDirector WRITE setSelectedDirector NOTIFY selectedDirectorChanged)
        Q_PROPERTY(QString selectedProducer READ selectedProducer WRITE setSelectedProducer NOTIFY selectedProducerChanged)

public:
    Navigator(QObject* parent = nullptr);

    // Getters
    QList<QVariantMap> collectionsData() const;
    QList<QVariantMap> mediaData() const;
    QList<QVariantMap> filteredData() const;
    QMap<QString, QVariantMap> mediaMetadata() const;

    
    QString currentCategory() const;
    QString selectedCollectionId() const;
    QList<QString> selectedGenres() const;
    QList<QString> selectedEras() const;
    bool showTopRated() const;
    QString selectedDirector() const;
    QString selectedProducer() const;
    bool groupByCollection() const;

    // Setters
    void setCurrentCategory(const QString& category);
    void setSelectedCollectionId(const QString& collectionId);
    void setSelectedGenres(const QList<QString>& genres);
    void setSelectedEras(const QList<QString>& eras);
    void setShowTopRated(bool showTopRated);
    void setSelectedDirector(const QString& director);
    void setSelectedProducer(const QString& producer);
    void setGroupByCollection(bool groupByCollection);

    // Methods
    Q_INVOKABLE QVariantList sidebarCategories() const;
    Q_INVOKABLE QString getMediaTitle(QString mediaId) const;
    Q_INVOKABLE QVariantList getUniqueGenres();
    Q_INVOKABLE QStringList getUniqueProducers();
    Q_INVOKABLE void clearFilters();
    Q_INVOKABLE QString getNextEpisode(QString currentMediaId, int index);
    Q_INVOKABLE float getMediaProgress(QString mediaId);
    Q_INVOKABLE void updateFilteredData();
    Q_INVOKABLE void updateFilteredData(QString searchText);
    Q_INVOKABLE void setMediaData(QList<QVariantMap> mediaData);
    Q_INVOKABLE void setCollectionsData(QList<QVariantMap> collectionsData);
    Q_INVOKABLE void setMediaMetadata(QList<QVariantMap> mediaMetadata);
    Q_INVOKABLE QVariantMap getMediaMetadata(QString mediaId);

signals:
    void collectionsDataChanged();
    void mediaDataChanged();
    void filteredDataChanged();
    void mediaMetadataChanged();
    void currentCategoryChanged();
    void selectedCollectionIdChanged();
    void selectedGenresChanged();
    void selectedErasChanged();
    void showTopRatedChanged();
    void groupByCollectionChanged();
    void selectedDirectorChanged();
    void selectedProducerChanged();

    void mediaLoaded();

private:
    QList<QVariantMap> m_collectionsData;
    QList<QVariantMap> m_mediaData;
    QMap<QString, QVariantMap> m_mediaMetadata;
    QList<QVariantMap> m_filteredData;

    QString m_currentCategory;
    QString m_selectedCollectionId;

    QList<QString> m_selectedGenres;
    QList<QString> m_selectedEras;
    bool m_showTopRated = false;
    bool m_groupByCollection = true;
    QString m_selectedDirector;
    QString m_selectedProducer;

    QMap<QString, QString> m_sidebarCategories;

    
    QSet<QString> parseJson(const QString& json);
    bool anyValueContains(const QVariantMap& map, const QString& substring) const;

    QVariantMap getCollection(QString collectionId);

    QString getEraFromYear(int year);
    QSet<QString> parseGenres(const QString& genresJson);

    QList<QVariantMap> getMediaByCollection(QString collectionId);
    bool mediaInCollection(QString mediaId, QString collectionId);

    void filterBySearchText(QList<QVariantMap>& filteredMedia, QList<QVariantMap>& filteredCollections, QString searchText);
    void filterByCategory(QList<QVariantMap>& filteredMedia, QList<QVariantMap>& filteredCollections);
    void filterByFilterBarMedia(QList<QVariantMap>& filteredMedia);
    void filterByFilterBarCollection(QList<QVariantMap>& filteredCollections);
    void filterByChosenCollection(QList<QVariantMap>& filteredMedia, QList<QVariantMap>& filteredCollections);
};

