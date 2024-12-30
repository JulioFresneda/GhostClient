#pragma once

#include <QObject>
#include <QString>
#include <QList>
#include <QVariant>

/**
 * @class Navigator
 * @brief Handles navigation and filtering logic for a media streaming client application.
 *
 * This class provides properties, methods, and signals to manage collections, media data,
 * and user selections for filtering and sorting media items.
 */
class Navigator : public QObject {
    Q_OBJECT

        // Properties exposed to QML
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
        Q_PROPERTY(QString sortBy READ sortBy WRITE setSortBy NOTIFY sortByChanged)
        Q_PROPERTY(bool sortOrder READ sortOrder WRITE setSortOrder NOTIFY sortOrderChanged)

public:
    /**
     * @brief Constructs a Navigator instance.
     * @param parent Pointer to the parent QObject.
     */
    Navigator(QObject* parent = nullptr);

    // Getters for properties
    QList<QVariantMap> collectionsData() const; ///< Returns the list of collection data.
    QList<QVariantMap> mediaData() const; ///< Returns the list of media data.
    QList<QVariantMap> filteredData() const; ///< Returns the filtered media data.
    QMap<QString, QVariantMap> mediaMetadata() const; ///< Returns metadata of media items.

    QString currentCategory() const; ///< Returns the current category selected by the user.
    QString selectedCollectionId() const; ///< Returns the ID of the selected collection.
    QList<QString> selectedGenres() const; ///< Returns the list of selected genres.
    QList<QString> selectedEras() const; ///< Returns the list of selected eras.
    bool showTopRated() const; ///< Indicates whether top-rated media should be shown.
    QString selectedDirector() const; ///< Returns the selected director's name.
    QString selectedProducer() const; ///< Returns the selected producer's name.
    bool groupByCollection() const; ///< Indicates whether media is grouped by collection.

    QString sortBy() const; ///< Returns the current sorting criteria.
    bool sortOrder() const; ///< Indicates the current sorting order (ascending/descending).

    // Setters for properties
    void setCurrentCategory(const QString& category); ///< Sets the current category.
    void setSelectedCollectionId(const QString& collectionId); ///< Sets the selected collection ID.
    void setSelectedGenres(const QList<QString>& genres); ///< Sets the selected genres.
    void setSelectedEras(const QList<QString>& eras); ///< Sets the selected eras.
    void setShowTopRated(bool showTopRated); ///< Sets whether to show top-rated media.
    void setSelectedDirector(const QString& director); ///< Sets the selected director.
    void setSelectedProducer(const QString& producer); ///< Sets the selected producer.
    void setGroupByCollection(bool groupByCollection); ///< Sets grouping by collection.
    void setSortBy(const QString& sortBy); ///< Sets the sorting criteria.
    void setSortOrder(const bool sortOrder); ///< Sets the sorting order.

    // QML-invokable methods
    Q_INVOKABLE QVariantList sidebarCategories() const; ///< Returns the list of sidebar categories.
    Q_INVOKABLE QString getPreviousCategory() const; ///< Returns the previous category.
    Q_INVOKABLE QString getNextCategory() const; ///< Returns the next category.
    Q_INVOKABLE QString getMediaTitle(QString mediaId) const; ///< Retrieves the title of a media item by its ID.
    Q_INVOKABLE QVariantList getUniqueGenres(); ///< Returns a list of unique genres.
    Q_INVOKABLE QStringList getUniqueProducers(); ///< Returns a list of unique producers.
    Q_INVOKABLE void clearFilters(); ///< Clears all applied filters.
    Q_INVOKABLE QString getFinalEpisode(); ///< Retrieves the last episode of a series.
    Q_INVOKABLE QString getNextEpisode(QString currentMediaId, int index); ///< Retrieves the next episode based on the current media ID and index.
    Q_INVOKABLE float getMediaProgress(QString mediaId); ///< Retrieves the progress of a media item by its ID.
    Q_INVOKABLE void updateFilteredData(); ///< Updates filtered data based on current filters.
    Q_INVOKABLE void updateFilteredData(QString searchText); ///< Updates filtered data based on a search text.
    Q_INVOKABLE void setMediaData(QList<QVariantMap> mediaData); ///< Sets the media data.
    Q_INVOKABLE void setCollectionsData(QList<QVariantMap> collectionsData); ///< Sets the collection data.
    Q_INVOKABLE void setMediaMetadata(QList<QVariantMap> mediaMetadata); ///< Sets the media metadata.
    Q_INVOKABLE QVariantMap getMediaMetadata(QString mediaId); ///< Retrieves metadata for a specific media item by its ID.
    Q_INVOKABLE QVariantMap getMedia(QString mediaId); ///< Retrieves media data by its ID.
    Q_INVOKABLE QString getEpisodeType(QString currentMediaId); ///< Determines the type of an episode.
    Q_INVOKABLE QString getCollectionId(QString mediaId); ///< Retrieves the collection ID for a media item.

signals:
    // Signals emitted when properties change
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
    void sortByChanged();
    void sortOrderChanged();
    void mediaLoaded(); ///< Signal emitted when media data is fully loaded.

private:
    // Private member variables
    QList<QVariantMap> m_collectionsData; ///< Holds the collection data.
    QList<QVariantMap> m_mediaData; ///< Holds the media data.
    QMap<QString, QVariantMap> m_mediaMetadata; ///< Maps media IDs to their metadata.
    QList<QVariantMap> m_filteredData; ///< Stores the filtered media data.

    QString m_currentCategory; ///< Stores the current category.
    QString m_selectedCollectionId; ///< Stores the selected collection ID.

    QList<QString> m_selectedGenres; ///< Stores the selected genres.
    QList<QString> m_selectedEras; ///< Stores the selected eras.
    bool m_showTopRated = false; ///< Indicates whether top-rated media is shown.
    bool m_groupByCollection = true; ///< Indicates grouping by collection.
    QString m_selectedDirector; ///< Stores the selected director.
    QString m_selectedProducer; ///< Stores the selected producer.
    QString m_sortBy; ///< Stores the sorting criteria.
    bool m_sortOrder; ///< Stores the sorting order.
    QMap<QString, QString> m_sidebarCategories; ///< Maps categories to their names.

    // Helper methods for internal use
    QSet<QString> parseJson(const QString& json); ///< Parses a JSON string into a set of strings.
    bool anyValueContains(const QVariantMap& map, const QString& substring) const; ///< Checks if any value in the map contains the given substring.

    QVariantMap getCollection(QString collectionId); ///< Retrieves a collection by its ID.

    QString getEraFromYear(int year); ///< Determines the era from a given year.
    QSet<QString> parseGenres(const QString& genresJson); ///< Parses a JSON string of genres into a set of strings.

    QList<QVariantMap> getMediaByCollection(QString collectionId); ///< Retrieves media items belonging to a collection.
    bool mediaInCollection(QString mediaId, QString collectionId); ///< Checks if a media item belongs to a collection.

    void filterBySearchText(QList<QVariantMap>& filteredMedia, QList<QVariantMap>& filteredCollections, QString searchText); ///< Filters media and collections by a search text.
    void filterByCategory(QList<QVariantMap>& filteredMedia, QList<QVariantMap>& filteredCollections); ///< Filters media and collections by category.
    void filterByFilterBarMedia(QList<QVariantMap>& filteredMedia); ///< Applies media filters from the filter bar.
    void filterByFilterBarCollection(QList<QVariantMap>& filteredCollections); ///< Applies collection filters from the filter bar.
    void filterByChosenCollection(QList<QVariantMap>& filteredMedia, QList<QVariantMap>& filteredCollections); ///< Filters media by the chosen collection.

    void sortByString(QList<QVariantMap>& list, QString sortby_media, QString sortby_collection); ///< Sorts a list by a string property.
    void sortByDouble(QList<QVariantMap>& list, QString sortby_media, QString sortby_collection); ///< Sorts a list by a numeric property.
    void sortByYear(QList<QVariantMap>& list); ///< Sorts a list by year.

    void sortBySeasonAndEpisode(QList<QVariantMap>& list); ///< Sorts a list by season and episode.
    void sortFilteredData(); ///< Sorts the filtered data.

    int getYearOfCollection(QString collectionId); ///< Retrieves the year of a collection by its ID.
};
