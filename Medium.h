// Login.h
#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QSettings>

class Medium : public QObject {
    Q_OBJECT
        Q_PROPERTY(bool hasStoredPassword READ hasStoredPassword NOTIFY hasStoredPasswordChanged)
        Q_PROPERTY(QString storedPassword READ getStoredPassword CONSTANT)
        Q_PROPERTY(QString userID READ getUserID CONSTANT)

public:
    Q_INVOKABLE QString getToken();
    explicit Medium(QObject* parent = nullptr);

    QString getStoredPassword() const { return m_storedPassword; }
    QString getUserID() const { return m_userID; }

    bool hasStoredPassword() const;
    Q_INVOKABLE void verifyLogin(const QString& token, const QString& userID);
    Q_INVOKABLE void addProfile(const QString& profileID, int pictureID);
    Q_INVOKABLE void selectProfile(const QString& profileID);

    Q_INVOKABLE void fetchUserProfile();
    Q_INVOKABLE void fetchMediaData();
    Q_INVOKABLE void fetchMediaMetadata();

public slots:
    void loadCoverImage(const QString& mediaId, const QString& backupId);

signals:
    void hasStoredPasswordChanged();
    void profileDataFetched(const QVariantList& profileData);
    void profileAdded();
    void profileSelected();
    void mediaDataFetched(const QVariantMap& mediaData);
    void mediaMetadataFetched(const QVariantList& mediaMetadata);
    void coverImageLoaded(const QString& mediaId, const QString& base64Data);
    void coverImageError(const QString& mediaId);

private:
    QNetworkAccessManager m_networkManager;
    QString m_storedPassword;
    QString m_token;
    QString m_userID;
    QString m_selectedProfileID;
    QString m_url;

    QString getBase64ImageFromServer(const QString& mediaId);

    void authenticate();
};