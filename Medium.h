/**
 * @file Medium.h
 * @brief Header file for the Medium class that handles media streaming client operations
 *
 * This class manages authentication, profile operations, and media data retrieval
 * for a QML-based media streaming client. It provides interfaces for user authentication,
 * profile management, and media content access.
 */

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QSettings>

 /**
  * @brief The Medium class handles media streaming client functionality
  *
  * Provides interfaces for user authentication, profile management,
  * and media content retrieval. Implements network operations for
  * communicating with the media streaming server.
  */
class Medium : public QObject {
    Q_OBJECT
        /**
         * @brief Property indicating whether user credentials are stored
         */
        Q_PROPERTY(bool hasStoredPassword READ hasStoredPassword NOTIFY hasStoredPasswordChanged)
        /**
         * @brief Property containing the stored user password
         */
        Q_PROPERTY(QString storedPassword READ getStoredPassword CONSTANT)
        /**
         * @brief Property containing the user identifier
         */
        Q_PROPERTY(QString userID READ getUserID CONSTANT)

public:
    /**
     * @brief Retrieves the current authentication token
     * @return QString Current JWT authentication token
     */
    Q_INVOKABLE QString getToken();

    /**
     * @brief Constructor for Medium class
     * @param parent Parent QObject for memory management
     */
    explicit Medium(QObject* parent = nullptr);

    /**
     * @brief Getter for stored password
     * @return QString The stored password
     */
    QString getStoredPassword() const { return m_storedPassword; }

    /**
     * @brief Getter for user ID
     * @return QString The user identifier
     */
    QString getUserID() const { return m_userID; }

    /**
     * @brief Checks if password is stored in settings
     * @return bool True if password exists, false otherwise
     */
    bool hasStoredPassword() const;

    /**
     * @brief Verifies and stores login credentials
     * @param token User's authentication token
     * @param userID User's identifier
     */
    Q_INVOKABLE void verifyLogin(const QString& token, const QString& userID);

    /**
     * @brief Creates a new user profile
     * @param profileID Identifier for the new profile
     * @param pictureID Associated picture identifier
     */
    Q_INVOKABLE void addProfile(const QString& profileID, int pictureID);

    /**
     * @brief Sets the active user profile
     * @param profileID Identifier of the profile to select
     */
    Q_INVOKABLE void selectProfile(const QString& profileID);

    /**
     * @brief Retrieves user profile data from server
     */
    Q_INVOKABLE void fetchUserProfile();

    /**
     * @brief Retrieves media content data from server
     */
    Q_INVOKABLE void fetchMediaData();

    /**
     * @brief Retrieves metadata for media content
     */
    Q_INVOKABLE void fetchMediaMetadata();

public slots:
    /**
     * @brief Loads a cover image with fallback support
     * @param mediaId Primary identifier for the cover image
     * @param backupId Backup identifier used if primary image fails
     */
    void loadCoverImage(const QString& mediaId, const QString& backupId);

signals:
    /**
     * @brief Emitted when stored password status changes
     */
    void hasStoredPasswordChanged();

    /**
     * @brief Emitted when profile data is successfully retrieved
     * @param profileData List of profile information
     */
    void profileDataFetched(const QVariantList& profileData);

    /**
     * @brief Emitted when a new profile is successfully created
     */
    void profileAdded();

    /**
     * @brief Emitted when a profile is selected as active
     */
    void profileSelected();

    /**
     * @brief Emitted when media content data is retrieved
     * @param mediaData Map containing media content information
     */
    void mediaDataFetched(const QVariantMap& mediaData);

    /**
     * @brief Emitted when media metadata is retrieved
     * @param mediaMetadata List of media metadata
     */
    void mediaMetadataFetched(const QVariantList& mediaMetadata);

    /**
     * @brief Emitted when a cover image is successfully loaded
     * @param mediaId Identifier of the media item
     * @param base64Data Base64 encoded image data
     */
    void coverImageLoaded(const QString& mediaId, const QString& base64Data);

    /**
     * @brief Emitted when cover image loading fails
     * @param mediaId Identifier of the media item that failed
     */
    void coverImageError(const QString& mediaId);

private:
    QNetworkAccessManager m_networkManager;  ///< Manages network requests
    QString m_storedPassword;                ///< User's stored password
    QString m_token;                         ///< Authentication token
    QString m_userID;                        ///< User identifier
    QString m_selectedProfileID;             ///< Currently selected profile
    QString m_url;                           ///< Server base URL

    /**
     * @brief Retrieves base64 encoded image from server
     * @param mediaId Identifier for the requested image
     * @return QString Base64 encoded image data
     */
    QString getBase64ImageFromServer(const QString& mediaId);

    /**
     * @brief Performs user authentication with stored credentials
     */
    void authenticate();
};