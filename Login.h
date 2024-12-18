// Login.h
#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QSettings>

class Login : public QObject {
    Q_OBJECT
        Q_PROPERTY(bool hasStoredToken READ hasStoredToken NOTIFY hasStoredTokenChanged)
        Q_PROPERTY(QString storedToken READ getStoredToken CONSTANT)
        Q_PROPERTY(QString userID READ getUserID CONSTANT)

public:
    explicit Login(QObject* parent = nullptr);

    QString getStoredToken() const { return m_storedToken; }
    QString getUserID() const { return m_userID; }

    bool hasStoredToken() const;
    Q_INVOKABLE void verifyLogin(const QString& token, const QString& userID);
    Q_INVOKABLE void addProfile(const QString& profileID, const QString& pictureID);
    Q_INVOKABLE void selectProfile(const QString& profileID);

    Q_INVOKABLE void fetchUserProfile();
    Q_INVOKABLE void fetchMediaData();
    Q_INVOKABLE void fetchMediaMetadata();

public slots:
    void loadCoverImage(const QString& mediaId, const QString& backupId);

signals:
    void hasStoredTokenChanged();
    void profileDataFetched(const QVariantList& profileData);
    void profileAdded();
    void profileSelected();
    void mediaDataFetched(const QVariantMap& mediaData);
    void mediaMetadataFetched(const QVariantList& mediaMetadata);
    void coverImageLoaded(const QString& mediaId, const QString& base64Data);
    void coverImageError(const QString& mediaId);

private:
    QNetworkAccessManager m_networkManager;
    QString m_storedToken;
    QString m_userID;
    QString m_selectedProfileID;

    QString getBase64ImageFromServer(const QString& mediaId);
};