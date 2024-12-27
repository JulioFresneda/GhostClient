#include "VLCPlayerHandler.h"
#include <QCoreApplication>
#include <QDir>
#include <QSettings>
#include <QUrl>
#include <QUrlQuery>
#include <QDebug>
#include <QQuickWindow>
#include <QJsonObject>
#include <QJsonDocument>
#include <QNetworkRequest>
#ifdef Q_OS_WIN
#include <windows.h>
#endif

void vlcLogCallback(void* data, int level, const libvlc_log_t* ctx, const char* fmt, va_list args) {
    char buf[1024];
    vsnprintf(buf, sizeof(buf), fmt, args);
    qDebug() << "VLC Log [" << level << "]:" << buf;
}

VLCPlayerHandler::VLCPlayerHandler(QObject* parent)
    : QObject(parent)
    , m_vlcInstance(nullptr)
    , m_mediaPlayer(nullptr)
    , m_media(nullptr)
    , m_isPlaying(false)
    , m_videoSink(nullptr)
{
    QSettings settings("./conf.ini", QSettings::IniFormat);
    m_token = settings.value("token").toString();
    m_profileId = settings.value("selectedProfileID").toString();
    QString port = settings.value("port").toString();
    m_url = "http://" + settings.value("publicIP").toString() + ":" + port;
    const char* args[] = {
        //"--no-video-title-show",
        //"--clock-jitter=0",
        //"--no-mouse-events",
        //"--input-fast-seek",
        //"--network-caching=1000",
        //"--adaptive-maxwidth=1920",
        //"--verbose=0",
        "--quiet",
        //"--subpicture-height=3000"
        //"--file-logging",             // Enable file logging
        //"--logfile=C:\\Users\\julio\\Documents\\vlc-log.txt"
    };

    m_vlcInstance = libvlc_new(sizeof(args) / sizeof(*args), args);
    libvlc_set_log_verbosity(m_vlcInstance, 2);
    if (!m_vlcInstance) {
        qDebug() << "Failed to create VLC instance";
        return;
    }

    m_mediaPlayer = libvlc_media_player_new(m_vlcInstance);
    if (!m_mediaPlayer) {
        qDebug() << "Failed to create media player";
        libvlc_release(m_vlcInstance);
        m_vlcInstance = nullptr;
        return;
    }

    m_positionTimer = new QTimer(this);
    connect(m_positionTimer, &QTimer::timeout, this, &VLCPlayerHandler::updateMediaInfo);
    m_positionTimer->setInterval(100);

    m_metadataTimer = new QTimer(this);
    connect(m_metadataTimer, &QTimer::timeout, this, &VLCPlayerHandler::updateMediaMetadataOnServer);
    m_metadataTimer->setInterval(30000);


    libvlc_log_set(m_vlcInstance, vlcLogCallback, nullptr);
}

VLCPlayerHandler::~VLCPlayerHandler() {
    cleanupVLC();
}


bool VLCPlayerHandler::verifyVLCSetup() {
    if (!m_vlcInstance || !m_mediaPlayer) {
        QString error = "VLC setup verification failed: ";
        if (!m_vlcInstance) error += "VLC instance is null. ";
        if (!m_mediaPlayer) error += "Media player is null. ";

        qDebug() << error;
        emit errorOccurred(error);
        return false;
    }
    return true;
}

void VLCPlayerHandler::setPosition(qint64 position) {
    if (!m_mediaPlayer) return;

    // Get media duration
    libvlc_time_t duration = libvlc_media_player_get_length(m_mediaPlayer);

    // For DASH streaming, we need to use precise seeking
    float percentage = static_cast<float>(position) / duration;

    libvlc_media_player_set_position(m_mediaPlayer, percentage);

    qDebug() << "Seeking to position:" << position
        << "Duration:" << duration
        << "Percentage:" << percentage;

    //emit positionChanged(position);
}

void VLCPlayerHandler::playMedia(float percentage_watched = 0) {
    if (m_mediaPlayer) {
        libvlc_media_player_play(m_mediaPlayer);
        libvlc_state_t state = libvlc_media_player_get_state(m_mediaPlayer);
        while (!(state == libvlc_Playing || state == libvlc_Paused)) {
            state = libvlc_media_player_get_state(m_mediaPlayer);;
        }
        m_isPlaying = true;
        if (percentage_watched > 0.0) {
            libvlc_media_player_set_position(m_mediaPlayer, percentage_watched);
            libvlc_time_t currentTime = libvlc_media_player_get_time(m_mediaPlayer);
            libvlc_time_t duration = libvlc_media_player_get_length(m_mediaPlayer);
            float position = libvlc_media_player_get_position(m_mediaPlayer);
            emit positionChanged(currentTime);
        }
        while (!(state == libvlc_Playing || state == libvlc_Paused)) {
            state = libvlc_media_player_get_state(m_mediaPlayer);;
        }
        //m_isPlaying = true;
        m_positionTimer->start(); // Start the timer when playing
        m_metadataTimer->start();
        emit playingStateChanged(true);
    }
}

void VLCPlayerHandler::pauseMedia() {
    if (m_mediaPlayer) {
        libvlc_media_player_pause(m_mediaPlayer);
        m_isPlaying = false;
        m_positionTimer->stop(); // Stop the timer when paused
        emit playingStateChanged(false);

        // Emit one final position update
        emit positionChanged(libvlc_media_player_get_time(m_mediaPlayer));
    }
}

void VLCPlayerHandler::stop() {
    if (m_mediaPlayer) {
        libvlc_media_player_stop(m_mediaPlayer);
        m_isPlaying = false;
        m_positionTimer->stop(); // Stop the timer when stopped

        if (m_vlcWindow) {
            m_vlcWindow->close(); // Close the window and remove it from the screen
            delete m_vlcWindow;   // Free the memory used by the window
            m_vlcWindow = nullptr; // Prevent dangling pointers
        }

        emit playingStateChanged(false);

        // Reset position to 0
        emit positionChanged(0);
    }
}

void VLCPlayerHandler::forward30sec() {
    if (m_mediaPlayer) {
        libvlc_time_t current_time = libvlc_media_player_get_time(m_mediaPlayer);
        libvlc_time_t new_time = current_time + 30000;
        libvlc_media_player_set_time(m_mediaPlayer, new_time);
        // Emit one final position update
        emit positionChanged(libvlc_media_player_get_time(m_mediaPlayer));
    }
}
void VLCPlayerHandler::back30sec() {
    if (m_mediaPlayer) {
        libvlc_time_t current_time = libvlc_media_player_get_time(m_mediaPlayer);
        libvlc_time_t new_time = current_time - 30000;
        libvlc_media_player_set_time(m_mediaPlayer, new_time);
        // Emit one final position update
        emit positionChanged(libvlc_media_player_get_time(m_mediaPlayer));
    }
}

void VLCPlayerHandler::updateMediaInfo() {
    if (!m_mediaPlayer) return;

    libvlc_state_t state = libvlc_media_player_get_state(m_mediaPlayer);
    if (state == libvlc_Playing || state == libvlc_Paused) {
        libvlc_time_t currentTime = libvlc_media_player_get_time(m_mediaPlayer);
        libvlc_time_t duration = libvlc_media_player_get_length(m_mediaPlayer);
        float position = libvlc_media_player_get_position(m_mediaPlayer);

        //qDebug() << "Media Info - Time:" << currentTime
        //    << "Duration:" << duration
        //    << "Position:" << position;

        if (currentTime >= 0 && duration > 0) {
            emit positionChanged(currentTime);

            
            
        }
    }
    else if (state == libvlc_Ended) {
        updateMediaMetadataOnServer();
        emit mediaEnded();
    }
}

void VLCPlayerHandler::updateMediaMetadata() {
    updateMediaMetadataOnServer();
}

void VLCPlayerHandler::updateMediaMetadataOnServer() {
    if (!m_mediaPlayer) return;

    QUrl url(m_url + "/update_media_metadata");
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("Authorization", "Bearer " + m_token.toUtf8());

    // Create the JSON payload
    QJsonObject jsonPayload;

    jsonPayload["profileID"] = m_profileId;
    jsonPayload["mediaID"] = m_currentMediaId;
    libvlc_state_t state = libvlc_media_player_get_state(m_mediaPlayer);
    float position = 1.0;
    if (state != libvlc_Ended) {
        position = libvlc_media_player_get_position(m_mediaPlayer);
    }
    
    jsonPayload["percentageWatched"] = QString::number(position, 'f', 3);
    jsonPayload["languageChosen"] = m_currentAudioText;
    jsonPayload["subtitlesChosen"] = m_currentSubtitlesText;  // Default subtitles

    QJsonDocument doc(jsonPayload);
    QByteArray jsonData = doc.toJson();

    // Send the POST request
    QNetworkReply* reply = m_networkManager.post(request, jsonData);

    // Handle the response
    connect(reply, &QNetworkReply::finished, this, [reply]() {
        if (reply->error() == QNetworkReply::NoError) {
            qDebug() << "Successfully updated media metadata";
        }
        else {
            qDebug() << "Error updating media metadata:" << reply->errorString();
        }
        reply->deleteLater();
        });
}

void VLCPlayerHandler::cleanupVLC() {
    if (m_media) {
        libvlc_media_release(m_media);
    }
    if (m_mediaPlayer) {
        libvlc_media_player_release(m_mediaPlayer);
    }
    if (m_vlcInstance) {
        libvlc_release(m_vlcInstance);
    }
}

qint64 VLCPlayerHandler::duration() const {
    return m_mediaPlayer ? (qint64)libvlc_media_player_get_length(m_mediaPlayer) : 0;
}

qint64 VLCPlayerHandler::position() const {
    return m_mediaPlayer ? (qint64)libvlc_media_player_get_time(m_mediaPlayer) : 0;
}

bool VLCPlayerHandler::isPlaying() const {
    return m_isPlaying;
}

void VLCPlayerHandler::setVolume(int volume) {
    libvlc_audio_set_volume(m_mediaPlayer, volume);
}

QVideoSink* VLCPlayerHandler::videoSink() const {
    return m_videoSink;
}

void VLCPlayerHandler::setVideoSink(QVideoSink* sink) {
    if (m_videoSink == sink)
        return;

    m_videoSink = sink;

    if (!m_videoSink || !m_mediaPlayer)
        return;

    auto videoOutput = m_videoSink->parent();
    if (!videoOutput || !videoOutput->inherits("QQuickItem"))
        return;

    QQuickItem* quickVideoOutput = qobject_cast<QQuickItem*>(videoOutput);
    if (!quickVideoOutput || !quickVideoOutput->window())
        return;

    if (!quickVideoOutput->window()->isVisible()) {
        QMetaObject::Connection* connection = new QMetaObject::Connection;
        *connection = connect(quickVideoOutput->window(), &QWindow::visibleChanged, this,
            [this, quickVideoOutput, connection](bool visible) {
                if (visible) {
                    QObject::disconnect(*connection);
                    delete connection;
                    attachVideoOutput(quickVideoOutput);
                }
            });
    }
    else {
        attachVideoOutput(quickVideoOutput);
    }

    emit videoSinkChanged();
}

// In your VLCPlayerHandler class
void VLCPlayerHandler::attachVideoOutput(QQuickItem* videoOutput) {
    if (!videoOutput || !videoOutput->window())
        return;

    // Create a child window that will host the VLC content
    m_vlcWindow = new QWindow(videoOutput->window());

    // Get the main window's size and global position
    mainWindow = videoOutput->window();
    QRect mainRect = mainWindow->geometry();
    QPointF pos = mainWindow->position();

    // Set initial geometry to full size minus 160 pixels height
    int adjustedHeight = mainRect.height() - 160;
    if (adjustedHeight < 0) {
        qWarning() << "Adjusted height is negative, setting to minimum height of 10.";
        adjustedHeight = 10; // Prevent negative or zero height
    }

    // Set the child window's geometry
    m_vlcWindow->setGeometry(0, 0, mainRect.width(), adjustedHeight);
    m_vlcWindow->setFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
    m_vlcWindow->show();

    QCoreApplication::instance()->installEventFilter(this);


    // Set VLC to render to this child window
#ifdef Q_OS_WIN
    WId handle = m_vlcWindow->winId();
    if (handle) {
        libvlc_media_player_set_hwnd(m_mediaPlayer, (void*)handle);
    }
#elif defined(Q_OS_LINUX)
    libvlc_media_player_set_xwindow(m_mediaPlayer, m_vlcWindow->winId());
#elif defined(Q_OS_MAC)
    libvlc_media_player_set_nsobject(m_mediaPlayer, (void*)m_vlcWindow->winId());
#endif

    libvlc_video_set_scale(m_mediaPlayer, 0); // Disable scaling; let geometry control rendering

    
}

bool VLCPlayerHandler::eventFilter(QObject* obj, QEvent* event) {
    // Only proceed if we're in full-screen mode
    if (fullScreen) {
        // Handle key press events
        if (event->type() == QEvent::KeyPress) {
            QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);
            if (keyEvent->key() == Qt::Key_Escape) {
                // Exit full-screen mode when the Escape key is pressed
                setFullScreen(false);
                return true; // Event is handled
            }
        }
        
    }

    // Pass the event on if it's not handled here
    return QObject::eventFilter(obj, event);
}





void VLCPlayerHandler::setFullScreen(bool setFullScreen) {
    if (!m_vlcWindow)
        return;
    fullScreen = setFullScreen;
    if (fullScreen) {
        // Set the child window to full screen
        QRect mainRect = mainWindow->geometry();
        int adjustedHeight = mainRect.height();

        m_vlcWindow->setGeometry(0, 0, mainRect.width(), adjustedHeight);
    }
    else {
        // Set the child window back to original size
        QRect mainRect = mainWindow->geometry();
        int adjustedHeight = mainRect.height()-160;

        m_vlcWindow->setGeometry(0, 0, mainRect.width(), adjustedHeight);
    }

    m_vlcWindow->show();
}




// In VLCPlayerHandler.cpp:
void VLCPlayerHandler::loadMedia(const QString& mediaId, const QVariantMap& mediaMetadata) {
    if (!verifyVLCSetup()) {
        return;
    }
    float percentage_watched = 0;
    if (!mediaMetadata.isEmpty()) {
        percentage_watched = mediaMetadata.value("percentage_watched").toFloat();
    }
    fullScreen = false;
    
    m_currentMediaId = mediaId;
    m_subtitleTracks.clear();

    

    if (m_media) {
        libvlc_media_release(m_media);
        m_media = nullptr;
    }

    QString baseUrl = QString(m_url + "/media/%1/manifest")
        .arg(mediaId);


    QByteArray urlBytes = baseUrl.toUtf8();
    m_media = libvlc_media_new_location(m_vlcInstance, urlBytes.constData());

    if (m_media) {
        libvlc_media_add_option(m_media, ":network-caching=1000");
        libvlc_media_add_option(m_media, ":http-reconnect");
        //libvlc_media_add_option(m_media, ":demux=adaptive");
        libvlc_media_add_option(m_media, ":adaptive-formats=dash");

        

        libvlc_media_player_set_media(m_mediaPlayer, m_media);

        tryDownloadSubtitles(mediaId);
        
        playMedia(percentage_watched);

        
        loadSubtitleTracks(mediaMetadata.value("subtitles_chosen").toString());
        loadAudioTracks(mediaMetadata.value("language_chosen").toString());
        
        emit subtitleTracksChanged();
        emit mediaLoaded();
        libvlc_time_t duration = libvlc_media_player_get_length(m_mediaPlayer);
        emit durationChanged(duration);



    }
    else {
        qDebug() << "Failed to create media";
        emit errorOccurred("Failed to create media");
    }
}



bool VLCPlayerHandler::tryDownloadSubtitles(const QString& mediaId) {

    QList<QString> languages;
    languages.append("es");
    languages.append("en");

    

    for (const QString& language : languages) {
        QUrl url(QString(m_url + "/media/%1/subtitles/%2").arg(mediaId, language + ".vtt"));

        // Check if the URL exists and returns 200
        QNetworkAccessManager manager;
        QNetworkRequest request(url);
        QEventLoop loop;

        QNetworkReply* reply = manager.get(request);
        QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
        loop.exec(); // Wait for the reply to finish

        int httpStatusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        if (reply->error() != QNetworkReply::NoError || httpStatusCode != 200) {
            qWarning() << "Subtitle URL does not exist or returned an error. Status code:" << httpStatusCode;
            reply->deleteLater();
            break;
        }

        reply->deleteLater();

        if (m_mediaPlayer) {
            // Store subtitle information
            QByteArray urlBytes = url.toString().toUtf8();
            const char* charurl = urlBytes.constData();

            qDebug() << charurl;
            libvlc_state_t state = libvlc_media_player_get_state(m_mediaPlayer);
            int result = libvlc_media_player_add_slave(
                m_mediaPlayer,
                libvlc_media_slave_type_subtitle,
                charurl,
                false
            );
            qDebug() << "Result of adding subtitle:" << result;

            if (result == 0) {
                qDebug() << "Added subtitle track:" << language;
                
            }
            else {
                qWarning() << "Failed to add subtitle track:" << language << "Error code:" << result;
                //return false;
            }
        }
        else {
            qWarning() << "Media player is not initialized.";
            return false;
        }
    }
    return true;
    
}

void VLCPlayerHandler::loadSubtitleTracks(QString subtitles_chosen) {
    m_subtitleTracks.clear();

    if (!m_mediaPlayer) return;

    m_currentSubtitlesId = libvlc_video_get_spu(m_mediaPlayer);
    int metadataSubtitleId = m_currentSubtitlesId;

    libvlc_track_description_t* tracks = libvlc_video_get_spu_description(m_mediaPlayer);
    libvlc_track_description_t* currentTrack = tracks;

    while (currentTrack) {
        QVariantMap trackInfo;
        trackInfo["id"] = currentTrack->i_id;
        trackInfo["name"] = QString::fromUtf8(currentTrack->psz_name);
        if (trackInfo["id"] == m_currentSubtitlesId) {
            m_currentSubtitlesText = trackInfo["name"].toString();
        }
        else if (trackInfo["name"] == subtitles_chosen) {
            metadataSubtitleId = trackInfo["id"].toInt();
        }
        if (trackInfo["id"] != -1) {
            m_subtitleTracks.append(trackInfo);
        }

        currentTrack = currentTrack->p_next;
    }

    if (tracks) {
        libvlc_track_description_list_release(tracks);
    }
    if (metadataSubtitleId != m_currentSubtitlesId) {
        setSubtitleTrack(metadataSubtitleId);
    }
    reorderListById(m_subtitleTracks, m_currentSubtitlesId);
    emit subtitleTracksChanged();

}

void VLCPlayerHandler::setSubtitleTrack(int trackId) {
    if (!m_mediaPlayer) return;

    libvlc_video_set_spu(m_mediaPlayer, trackId);
    qDebug() << "Setting subtitles track to:" << trackId;
    updateSubtitleSelected();
}

void VLCPlayerHandler::disableSubtitles() {
    if (m_mediaPlayer) {
        libvlc_video_set_spu(m_mediaPlayer, -1);
    }
}

QVariantList VLCPlayerHandler::subtitleTracks() const {
    return m_subtitleTracks;
}


QVariantList VLCPlayerHandler::audioTracks() const {
    return m_audioTracks;
}

void VLCPlayerHandler::reorderListById(QVariantList& list, int selectedTrackId) {
    // Find the index of the QVariantMap with the given "id"
    int targetIndex = -1;
    for (int i = 0; i < list.size(); ++i) {
        QVariantMap map = list[i].toMap();
        if (map["id"].toInt() == selectedTrackId) {
            targetIndex = i;
            break;
        }
    }

    // If the target was found, move it to the front
    if (targetIndex != -1) {
        QVariant targetMap = list.takeAt(targetIndex); // Remove from original position
        list.prepend(targetMap); // Add to the front
    }
}

void VLCPlayerHandler::updateAudioSelected() {
    m_currentAudioId = libvlc_audio_get_track(m_mediaPlayer);

    libvlc_track_description_t* tracks = libvlc_audio_get_track_description(m_mediaPlayer);
    libvlc_track_description_t* currentTrack = tracks;

    while (currentTrack) {

        if (currentTrack->i_id == m_currentAudioId) {
            m_currentAudioText = QString::fromUtf8(currentTrack->psz_name);
        }
        currentTrack = currentTrack->p_next;
    }

    if (tracks) {
        libvlc_track_description_list_release(tracks);
    }
}

void VLCPlayerHandler::updateSubtitleSelected() {
    m_currentSubtitlesId = libvlc_video_get_spu(m_mediaPlayer);

    libvlc_track_description_t* tracks = libvlc_audio_get_track_description(m_mediaPlayer);
    libvlc_track_description_t* currentTrack = tracks;

    while (currentTrack) {

        if (currentTrack->i_id == m_currentSubtitlesId) {
            m_currentSubtitlesText = QString::fromUtf8(currentTrack->psz_name);
        }
        currentTrack = currentTrack->p_next;
    }

    if (tracks) {
        libvlc_track_description_list_release(tracks);
    }
}

void VLCPlayerHandler::loadAudioTracks(QString languageChosen) {
    m_audioTracks.clear();

    if (!m_mediaPlayer) return;

    m_currentAudioId = libvlc_audio_get_track(m_mediaPlayer);
    int metadataAudioId = m_currentAudioId;

    libvlc_track_description_t* tracks = libvlc_audio_get_track_description(m_mediaPlayer);
    libvlc_track_description_t* currentTrack = tracks;

    while (currentTrack) {
        QVariantMap trackInfo;
        trackInfo["id"] = currentTrack->i_id;
        trackInfo["name"] = QString::fromUtf8(currentTrack->psz_name);
        if (trackInfo["id"] == m_currentAudioId) {
            m_currentAudioText = trackInfo["name"].toString();
        }
        else if (trackInfo["name"] == languageChosen) {
            metadataAudioId = trackInfo["id"].toInt();
        }
        if (trackInfo["id"] != -1) {
            m_audioTracks.append(trackInfo);
        }
        
        currentTrack = currentTrack->p_next;
    }

    

    if (tracks) {
        libvlc_track_description_list_release(tracks);
    }
    if (metadataAudioId != m_currentAudioId) {
        setAudioTrack(metadataAudioId);
    }
    reorderListById(m_audioTracks, m_currentAudioId);
    emit audioTracksChanged();
}

void VLCPlayerHandler::setAudioTrack(int trackId) {
    if (!m_mediaPlayer) return;

    libvlc_audio_set_track(m_mediaPlayer, trackId);
    qDebug() << "Setting audio track to:" << trackId;
    updateAudioSelected();
}

int VLCPlayerHandler::getAudioIndex() {
    return m_currentAudioId;
}

QString VLCPlayerHandler::getAudioText() {
    return m_currentAudioText;
}


