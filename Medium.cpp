#include "Medium.h"
#include <QJsonObject>
#include <QJsonDocument>
#include <QNetworkRequest>
#include <QCoreApplication>
#include <QJsonArray>


Medium::Medium(QObject* parent) : QObject(parent) {
    QSettings settings("./conf.ini", QSettings::IniFormat);
    m_storedPassword = settings.value("password", "").toString();
    m_userID = settings.value("userID", "").toString();
    m_url = "http://" + settings.value("publicIP").toString();
    authenticate();
    // If credentials are stored, fetch the profile data
    if (m_token != "") {
        fetchUserProfile();
    }
}

void Medium::authenticate() {
    if (m_storedPassword.isEmpty() || m_userID.isEmpty()) {
        qDebug() << "Error: Empty userID or password.";
        return;
    }

    // Define the URL for the login endpoint
    QUrl url(m_url + "/auth/login");
    qDebug() << m_url;
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    // Create the JSON payload with userID and password
    QJsonObject json;
    json["userID"] = m_userID;
    json["password"] = m_storedPassword;

    // Send a POST request with the JSON payload
    QNetworkReply* reply = m_networkManager.post(request, QJsonDocument(json).toJson());

    // Use a QEventLoop to block until the network request finishes
    QEventLoop loop;
    connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec(); // Blocks until the reply emits finished()

    // Handle the response
    if (reply->error() == QNetworkReply::NoError) {
        // Parse the server response
        QJsonDocument responseDoc = QJsonDocument::fromJson(reply->readAll());
        QJsonObject responseObj = responseDoc.object();

        // Extract the token from the response
        if (responseObj.contains("token")) {
            m_token = responseObj["token"].toString(); // Store the token for future requests
            qDebug() << "Authentication successful! Token:" << m_token;
        }
        else {
            qDebug() << "Authentication failed: Token not found in response.";
        }
    }
    else {
        // Handle errors
        qDebug() << "Error authenticating:" << reply->errorString();
    }

    reply->deleteLater(); // Clean up the reply object
}



bool Medium::hasStoredPassword() const {
    return !m_storedPassword.isEmpty();
}

void Medium::verifyLogin(const QString& password, const QString& userID) {
    if (password.isEmpty() || userID.isEmpty()) {
        qDebug() << "Password or User ID is missing.";
        return;
    }

    // Store credentials using QSettings
    QSettings settings("./conf.ini", QSettings::IniFormat);
    settings.setValue("password", password);
    settings.setValue("userID", userID);
    settings.sync();

    qDebug() << "Login verified with password:" << password << "and user ID:" << userID;

    // Exit the application after saving credentials
    QCoreApplication::exit();
}


void Medium::fetchUserProfile() {
    if (m_token.isEmpty()) return; // Ensure the token is available

    QUrl url(m_url + "/profile/list");
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("Authorization", "Bearer " + m_token.toUtf8()); // Add JWT in the Authorization header

    // Create an empty JSON object (userID and password are no longer sent)
    QJsonObject json;

    QNetworkReply* reply = m_networkManager.post(request, QJsonDocument(json).toJson());
    connect(reply, &QNetworkReply::finished, [this, reply]() {
        if (reply->error() == QNetworkReply::NoError) {
            // Parse the server response
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
        reply->deleteLater(); // Clean up the reply object
        });
}


void Medium::addProfile(const QString& profileID, const QString& pictureID) {
    if (m_storedPassword.isEmpty() || m_userID.isEmpty()) {
        qDebug() << "UserID or Token is missing.";
        return;
    }

    QUrl url(m_url + "/profile/add");
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("Authorization", "Bearer " + m_token.toUtf8()); // Add JWT in the Authorization header
    // Create JSON body
    QJsonObject json;

    json["profileID"] = profileID;
    json["pictureID"] = pictureID;


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

void Medium::selectProfile(const QString& profileID) {
    QSettings settings;
    settings.setValue("selectedProfileID", profileID);
    m_selectedProfileID = profileID;
    emit profileSelected(); 
}

void Medium::fetchMediaData() {
    QUrl url(m_url + "/download/media_data");
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("Authorization", "Bearer " + m_token.toUtf8()); // Add JWT in the Authorization header

    QJsonObject json;
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


void Medium::fetchMediaMetadata() {
    QUrl url(m_url + "/download/media_metadata");
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("Authorization", "Bearer " + m_token.toUtf8()); // Add JWT in the Authorization header

    QJsonObject json;
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

QString Medium::getBase64ImageFromServer(const QString& mediaId) {
    QUrl url(QString(m_url + "/cover/%1").arg(mediaId));
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("Authorization", "Bearer " + m_token.toUtf8()); // Add JWT in the Authorization header

    QJsonObject jsonObj;


    // Send the request
    QNetworkReply* reply = m_networkManager.post(request, QJsonDocument(jsonObj).toJson());

    // Use QEventLoop to wait synchronously for the reply to finish
    QEventLoop eventLoop;
    connect(reply, &QNetworkReply::finished, &eventLoop, &QEventLoop::quit);
    eventLoop.exec(); // Blocks until finished is emitted

    // Process the reply
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


// Login.cpp
void Medium::loadCoverImage(const QString& mediaId, const QString& backupId) {
    QString base64 = getBase64ImageFromServer(mediaId);
    if (base64 == "") {
        base64 = getBase64ImageFromServer(backupId);
    }
    if (base64 != "") {
        emit coverImageLoaded(mediaId, base64);
    }
    else {
        emit coverImageError(mediaId);
    }
}

QString Medium::getToken() {
    return m_token;
}
