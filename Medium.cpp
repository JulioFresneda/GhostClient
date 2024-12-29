/**
 * @file Medium.cpp
 * @brief Implementation of the Medium class for handling media streaming client operations
 *
 * This file contains the implementation of network operations, authentication, profile
 * management, and media data handling for a QML-based media streaming client.
 */

#include "Medium.h"
#include <QJsonObject>
#include <QJsonDocument>
#include <QNetworkRequest>
#include <QCoreApplication>
#include <QJsonArray>

 /**
  * @brief Constructor for the Medium class
  * @param parent Parent QObject for memory management
  */
Medium::Medium(QObject* parent) : QObject(parent) {
    // Initialize settings from configuration file
    QSettings settings("./conf.ini", QSettings::IniFormat);

    // Load stored credentials and server URL
    m_storedPassword = settings.value("password", "").toString();
    m_userID = settings.value("userID", "").toString();
    m_url = "http://" + settings.value("publicIP").toString() + ":38080";

    // Perform initial authentication
    authenticate();

    // Store the authentication token
    settings.setValue("token", m_token);
    settings.sync();

    // Fetch profile data if authentication token exists
    if (m_token != "") {
        fetchUserProfile();
    }
}

/**
 * @brief Authenticates the user with stored credentials
 */
void Medium::authenticate() {
    // Validate credentials exist
    if (m_storedPassword.isEmpty() || m_userID.isEmpty()) {
        qDebug() << "Error: Empty userID or password.";
        return;
    }

    // Prepare authentication request
    QUrl url(m_url + "/auth/login");
    qDebug() << m_url;
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    // Create authentication payload
    QJsonObject json;
    json["userID"] = m_userID;
    json["password"] = m_storedPassword;

    // Send authentication request
    QNetworkReply* reply = m_networkManager.post(request, QJsonDocument(json).toJson());

    // Wait for response using event loop
    QEventLoop loop;
    connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();

    // Process authentication response
    if (reply->error() == QNetworkReply::NoError) {
        QJsonDocument responseDoc = QJsonDocument::fromJson(reply->readAll());
        QJsonObject responseObj = responseDoc.object();

        // Extract and store authentication token
        if (responseObj.contains("token")) {
            m_token = responseObj["token"].toString();
            qDebug() << "Authentication successful! Token:" << m_token;
        }
        else {
            qDebug() << "Authentication failed: Token not found in response.";
        }
    }
    else {
        qDebug() << "Error authenticating:" << reply->errorString();
    }

    reply->deleteLater();
}

/**
 * @brief Checks if a password is stored in settings
 * @return bool True if a password exists, false otherwise
 */
bool Medium::hasStoredPassword() const {
    return !m_storedPassword.isEmpty();
}

/**
 * @brief Verifies login credentials and stores them
 * @param password User's password
 * @param userID User's ID
 */
void Medium::verifyLogin(const QString& password, const QString& userID) {
    // Validate input credentials
    if (password.isEmpty() || userID.isEmpty()) {
        qDebug() << "Password or User ID is missing.";
        return;
    }

    // Store credentials in configuration
    QSettings settings("./conf.ini", QSettings::IniFormat);
    settings.setValue("password", password);
    settings.setValue("userID", userID);
    settings.sync();

    qDebug() << "Login verified with password:" << password << "and user ID:" << userID;

    // Restart application to apply new credentials
    QCoreApplication::exit();
}

/**
 * @brief Fetches user profile data from the server
 */
void Medium::fetchUserProfile() {
    if (m_token.isEmpty()) return;

    // Prepare profile request
    QUrl url(m_url + "/profile/list");
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("Authorization", "Bearer " + m_token.toUtf8());

    // Send profile request
    QJsonObject json;
    QNetworkReply* reply = m_networkManager.post(request, QJsonDocument(json).toJson());

    // Handle response asynchronously
    connect(reply, &QNetworkReply::finished, [this, reply]() {
        if (reply->error() == QNetworkReply::NoError) {
            // Parse profile data from response
            QJsonDocument responseDoc = QJsonDocument::fromJson(reply->readAll());
            QJsonObject responseObj = responseDoc.object();
            QJsonArray profilesArray = responseObj["profiles"].toArray();

            // Convert profiles to QVariant format for QML
            QVariantList profiles;
            for (const QJsonValue& value : profilesArray) {
                QJsonObject obj = value.toObject();
                QVariantMap profile;
                profile["profileID"] = obj["profileID"].toString();
                profile["pictureID"] = obj["pictureID"].toString();
                profiles.append(profile);
            }

            emit profileDataFetched(profiles);
            qDebug() << "Profile fetched successfully.";
        }
        else {
            qDebug() << "Error fetching profile:" << reply->errorString();
        }
        reply->deleteLater();
        });
}

/**
 * @brief Adds a new profile to the user account
 * @param profileID Identifier for the new profile
 * @param pictureID Picture identifier for the profile
 */
void Medium::addProfile(const QString& profileID, int pictureID) {
    // Validate credentials exist
    if (m_storedPassword.isEmpty() || m_userID.isEmpty()) {
        qDebug() << "UserID or Token is missing.";
        return;
    }

    // Prepare profile creation request
    QUrl url(m_url + "/profile/add");
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("Authorization", "Bearer " + m_token.toUtf8());

    // Create profile payload
    QJsonObject json;
    json["profileID"] = profileID;
    json["pictureID"] = pictureID;

    // Send profile creation request
    QNetworkReply* reply = m_networkManager.post(request, QJsonDocument(json).toJson());
    connect(reply, &QNetworkReply::finished, [this, reply]() {
        if (reply->error() == QNetworkReply::NoError) {
            emit profileAdded();
            qDebug() << "Profile added successfully.";
        }
        else {
            qDebug() << "Error adding profile:" << reply->errorString();
        }
        reply->deleteLater();
        });
}

/**
 * @brief Selects a profile for the current session
 * @param profileID Identifier of the profile to select
 */
void Medium::selectProfile(const QString& profileID) {
    // Store selected profile in settings
    QSettings settings;
    settings.setValue("selectedProfileID", profileID);
    m_selectedProfileID = profileID;
    emit profileSelected();
}

/**
 * @brief Fetches media data for the selected profile
 */
void Medium::fetchMediaData() {
    // Prepare media data request
    QUrl url(m_url + "/download/media_data");
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("Authorization", "Bearer " + m_token.toUtf8());

    // Create request payload
    QJsonObject json;
    json["profileID"] = m_selectedProfileID;

    // Send media data request
    QNetworkReply* reply = m_networkManager.post(request, QJsonDocument(json).toJson());
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        if (reply->error() == QNetworkReply::NoError) {
            // Parse media data response
            QByteArray responseData = reply->readAll();
            QJsonDocument responseDoc = QJsonDocument::fromJson(responseData);

            if (!responseDoc.isNull()) {
                QJsonObject rootObj = responseDoc.object();
                QVariantMap mediaData;

                // Extract collections data
                if (rootObj.contains("collections")) {
                    mediaData["collections"] = rootObj["collections"].toArray().toVariantList();
                    qDebug() << "Collections loaded:" << mediaData["collections"].toList().count();
                }

                // Extract media items data
                if (rootObj.contains("media")) {
                    mediaData["media"] = rootObj["media"].toArray().toVariantList();
                    qDebug() << "Media items loaded:" << mediaData["media"].toList().count();
                }

                // Log and emit media data
                qDebug() << "Media data structure:";
                qDebug() << "Collections:" << mediaData["collections"].toList().count();
                qDebug() << "Media items:" << mediaData["media"].toList().count();
                emit mediaDataFetched(mediaData);
                qDebug() << "Emitting complete media data structure";
            }
        }
        else {
            qDebug() << "Network error:" << reply->errorString();
        }
        reply->deleteLater();
        });
}

/**
 * @brief Fetches metadata for media items
 */
void Medium::fetchMediaMetadata() {
    // Prepare metadata request
    QUrl url(m_url + "/download/media_metadata");
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("Authorization", "Bearer " + m_token.toUtf8());

    // Create request payload
    QJsonObject json;
    json["profileID"] = m_selectedProfileID;

    // Send metadata request
    QNetworkReply* reply = m_networkManager.post(request, QJsonDocument(json).toJson());
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        if (reply->error() == QNetworkReply::NoError) {
            // Parse metadata response
            QByteArray responseData = reply->readAll();
            QJsonDocument responseDoc = QJsonDocument::fromJson(responseData);
            QJsonObject rootObj = responseDoc.object();

            QVariantList mediaMetadata;

            // Extract metadata array
            if (rootObj.contains("mediaMetadata") && rootObj["mediaMetadata"].isArray()) {
                mediaMetadata = rootObj["mediaMetadata"].toArray().toVariantList();
            }

            // Log response for debugging
            qDebug() << "\n=== Media Metadata Response ===";
            QJsonDocument formattedDoc(rootObj);
            qDebug().noquote() << formattedDoc.toJson(QJsonDocument::Indented);
            qDebug() << "=== End Media Metadata Response ===\n";

            emit mediaMetadataFetched(mediaMetadata);
        }
        else {
            qDebug() << "Network error:" << reply->errorString();
            qDebug() << "HTTP status code:" << reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        }
        reply->deleteLater();
        });
}

/**
 * @brief Retrieves a base64 encoded image from the server
 * @param mediaId Identifier for the requested image
 * @return QString Base64 encoded image data or empty string on failure
 */
QString Medium::getBase64ImageFromServer(const QString& mediaId) {
    // Prepare image request
    QUrl url(QString(m_url + "/cover/%1").arg(mediaId));
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("Authorization", "Bearer " + m_token.toUtf8());

    QJsonObject jsonObj;

    // Send synchronous image request
    QNetworkReply* reply = m_networkManager.post(request, QJsonDocument(jsonObj).toJson());

    // Wait for response using event loop
    QEventLoop eventLoop;
    connect(reply, &QNetworkReply::finished, &eventLoop, &QEventLoop::quit);
    eventLoop.exec();

    // Process image response
    QString base64;
    if (reply->error() == QNetworkReply::NoError) {
        QByteArray imageData = reply->readAll();
        base64 = QString::fromLatin1(imageData.toBase64());
    }
    else {
        qWarning() << "Network error:" << reply->errorString();
        base64 = QString("");
    }

    reply->deleteLater();
    return base64;
}

/**
 * @brief Loads a cover image with fallback support
 * @param mediaId Primary identifier for the cover image
 * @param backupId Backup identifier used if primary image fails to load
 */
void Medium::loadCoverImage(const QString& mediaId, const QString& backupId) {
    // Try to load primary image
    QString base64 = getBase64ImageFromServer(mediaId);

    // Fall back to backup image if primary fails
    if (base64 == "") {
        base64 = getBase64ImageFromServer(backupId);
    }

    // Emit appropriate signal based on result
    if (base64 != "") {
        emit coverImageLoaded(mediaId, base64);
    }
    else {
        emit coverImageError(mediaId);
    }
}

/**
 * @brief Retrieves the current authentication token
 * @return QString Current JWT token
 */
QString Medium::getToken() {
    return m_token;
}