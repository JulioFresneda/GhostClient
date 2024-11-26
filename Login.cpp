#include "Login.h"
#include <QJsonObject>
#include <QJsonDocument>
#include <QNetworkRequest>
#include <QCoreApplication>
#include <QJsonArray>


Login::Login(QObject* parent) : QObject(parent) {
    QSettings settings("./conf.ini", QSettings::IniFormat);
    m_storedToken = settings.value("authToken", "").toString();
    m_userID = settings.value("userID", "").toString();


    // If credentials are stored, fetch the profile data
    if (!m_storedToken.isEmpty() && !m_userID.isEmpty()) {
        fetchUserProfile();
    }
}

bool Login::hasStoredToken() const {
    return !m_storedToken.isEmpty();
}

void Login::verifyLogin(const QString& token, const QString& userID) {
    if (token.isEmpty() || userID.isEmpty()) {
        qDebug() << "Token or User ID is missing.";
        return;
    }

    // Store credentials using QSettings
    QSettings settings("./conf.ini", QSettings::IniFormat);
    settings.setValue("authToken", token);
    settings.setValue("userID", userID);
    settings.sync();

    qDebug() << "Login verified with token:" << token << "and user ID:" << userID;

    // Exit the application after saving credentials
    QCoreApplication::exit();
}


void Login::fetchUserProfile() {
    if (m_storedToken.isEmpty() || m_userID.isEmpty()) return;

    QUrl url("http://localhost:18080/profile/list");
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QJsonObject json;
    json["token"] = m_storedToken;
    json["userID"] = m_userID;

    QNetworkReply* reply = m_networkManager.post(request, QJsonDocument(json).toJson());
    connect(reply, &QNetworkReply::finished, [this, reply]() {
        if (reply->error() == QNetworkReply::NoError) {
            QJsonDocument responseDoc = QJsonDocument::fromJson(reply->readAll());
            QJsonObject responseObj = responseDoc.object();
            QJsonArray profilesArray = responseObj["profiles"].toArray();

            QVariantList profiles;
            for (const QJsonValue& value : profilesArray) {
                QJsonObject obj = value.toObject();
                QVariantMap profile;
                profile["profileID"] = obj["profileID"].toString();
                profile["pictureID"] = obj["pictureID"].toString();
                profiles.append(profile);
            }

            emit profileDataFetched(profiles);  // Emit list of profile data
            qDebug() << "Profile fetched successfully.";
        }
        else {
            qDebug() << "Error fetching profile:" << reply->errorString();
        }
        reply->deleteLater();
        });
}

void Login::addProfile(const QString& profileID, const QString& pictureID) {
    if (m_storedToken.isEmpty() || m_userID.isEmpty()) {
        qDebug() << "UserID or Token is missing.";
        return;
    }

    QUrl url("http://localhost:18080/profile/add");
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    // Create JSON body
    QJsonObject json;
    json["userID"] = m_userID;
    json["profileID"] = profileID;
    json["pictureID"] = pictureID;
    json["token"] = m_storedToken;

    QNetworkReply* reply = m_networkManager.post(request, QJsonDocument(json).toJson());
    connect(reply, &QNetworkReply::finished, [this, reply]() {
        if (reply->error() == QNetworkReply::NoError) {
            emit profileAdded(); // Notify QML of success
            qDebug() << "Profile added successfully.";
        }
        else {
            qDebug() << "Error adding profile:" << reply->errorString();
        }
        reply->deleteLater();
        });
}

void Login::selectProfile(const QString& profileID) {
    QSettings settings;
    settings.setValue("selectedProfileID", profileID);
    m_selectedProfileID = profileID;
    emit profileSelected(); 
}

void Login::fetchMediaData() {
    QUrl url("http://localhost:18080/download/media_data");
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QJsonObject json;
    json["userID"] = m_userID;
    json["token"] = m_storedToken;
    json["profileID"] = m_selectedProfileID;

    QNetworkReply* reply = m_networkManager.post(request, QJsonDocument(json).toJson());
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        if (reply->error() == QNetworkReply::NoError) {
            QByteArray responseData = reply->readAll();
            QJsonDocument responseDoc = QJsonDocument::fromJson(responseData);

            if (!responseDoc.isNull()) {
                QJsonObject rootObj = responseDoc.object();

                // Create a QVariantMap to hold both collections and media
                QVariantMap mediaData;

                // Add collections array
                if (rootObj.contains("collections")) {
                    mediaData["collections"] = rootObj["collections"].toArray().toVariantList();
                    qDebug() << "Collections loaded:" << mediaData["collections"].toList().count();
                }

                // Add media array
                if (rootObj.contains("media")) {
                    mediaData["media"] = rootObj["media"].toArray().toVariantList();
                    qDebug() << "Media items loaded:" << mediaData["media"].toList().count();
                }

                // Emit the complete data structure
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


void Login::fetchMediaMetadata() {
    QUrl url("http://localhost:18080/download/media_metadata");
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QJsonObject json;
    json["userID"] = m_userID;
    json["token"] = m_storedToken;
    json["profileID"] = m_selectedProfileID;

    QNetworkReply* reply = m_networkManager.post(request, QJsonDocument(json).toJson());
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        if (reply->error() == QNetworkReply::NoError) {
            QByteArray responseData = reply->readAll();
            QJsonDocument responseDoc = QJsonDocument::fromJson(responseData);
            QJsonObject rootObj = responseDoc.object();

            QVariantList mediaMetadata; // Store the list of JSON objects

            // If "mediaMetadata" exists, extract it as a list of JSON objects
            if (rootObj.contains("mediaMetadata") && rootObj["mediaMetadata"].isArray()) {
                mediaMetadata = rootObj["mediaMetadata"].toArray().toVariantList();
            }

            qDebug() << "\n=== Media Metadata Response ===";
            QJsonDocument formattedDoc(rootObj);
            qDebug().noquote() << formattedDoc.toJson(QJsonDocument::Indented);
            qDebug() << "=== End Media Metadata Response ===\n";

            emit mediaMetadataFetched(mediaMetadata); // Emit the list directly
        }
        else {
            qDebug() << "Network error:" << reply->errorString();
            qDebug() << "HTTP status code:" << reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        }
        reply->deleteLater();
        });
}


// Login.cpp
void Login::loadCoverImage(const QString& mediaId) {
    QUrl url(QString("http://localhost:18080/cover/%1").arg(mediaId));
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QJsonObject jsonObj;
    jsonObj["token"] = m_storedToken;
    jsonObj["userID"] = m_userID;

    QNetworkReply* reply = m_networkManager.post(request, QJsonDocument(jsonObj).toJson());

    connect(reply, &QNetworkReply::finished, this, [=]() {
        reply->deleteLater();

        if (reply->error() == QNetworkReply::NoError) {
            QByteArray imageData = reply->readAll();
            QString base64 = QString::fromLatin1(imageData.toBase64());
            emit coverImageLoaded(mediaId, base64);
        }
        else {
            emit coverImageError(mediaId);
            //qDebug() << "Failed to load cover for ID:" << mediaId
            //    << "Error:" << reply->errorString();
        }
        });
}



