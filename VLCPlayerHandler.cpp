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

/**
 * @brief Callback function for VLC logging system
 * @param data User data pointer passed to the callback
 * @param level Severity level of the log message
 * @param ctx VLC log context
 * @param fmt Format string for the log message
 * @param args Variable argument list containing log message parameters
 */
void vlcLogCallback(void* data, int level, const libvlc_log_t* ctx, const char* fmt, va_list args) {
    char buf[1024];
    vsnprintf(buf, sizeof(buf), fmt, args);
    qDebug() << "VLC Log [" << level << "]:" << buf;
}

/**
 * @brief Constructs the VLCPlayerHandler with initial configuration
 * @param parent Parent QObject for memory management
 */
VLCPlayerHandler::VLCPlayerHandler(QObject* parent)
    : QObject(parent)
    , m_vlcInstance(nullptr)
    , m_mediaPlayer(nullptr)
    , m_media(nullptr)
    , m_isPlaying(false)
    , m_videoSink(nullptr)
{
    // Initialize configuration from settings file
    QSettings settings("./conf.ini", QSettings::IniFormat);
    m_token = settings.value("token").toString();
    m_profileId = settings.value("selectedProfileID").toString();
    QString port = settings.value("port").toString();
    m_url = "http://" + settings.value("publicIP").toString() + ":" + port;

    // VLC command line arguments
    const char* args[] = {
        "--quiet",
    };

    // Initialize VLC instance
    m_vlcInstance = libvlc_new(sizeof(args) / sizeof(*args), args);
    libvlc_set_log_verbosity(m_vlcInstance, 2);
    if (!m_vlcInstance) {
        qDebug() << "Failed to create VLC instance";
        return;
    }

    // Create media player instance
    m_mediaPlayer = libvlc_media_player_new(m_vlcInstance);
    if (!m_mediaPlayer) {
        qDebug() << "Failed to create media player";
        libvlc_release(m_vlcInstance);
        m_vlcInstance = nullptr;
        return;
    }

    // Initialize position update timer
    m_positionTimer = new QTimer(this);
    connect(m_positionTimer, &QTimer::timeout, this, &VLCPlayerHandler::updateMediaInfo);
    m_positionTimer->setInterval(100);

    // Initialize metadata update timer
    m_metadataTimer = new QTimer(this);
    connect(m_metadataTimer, &QTimer::timeout, this, &VLCPlayerHandler::updateMediaMetadataOnServer);
    m_metadataTimer->setInterval(30000);

    // Set up VLC logging
    libvlc_log_set(m_vlcInstance, vlcLogCallback, nullptr);
}

/**
 * @brief Destructor that ensures proper cleanup of VLC resources
 */
VLCPlayerHandler::~VLCPlayerHandler() {
    cleanupVLC();
}

/**
 * @brief Verifies that VLC components are properly initialized
 * @return bool True if setup is valid, false otherwise
 */
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

/**
 * @brief Sets the playback position of the media
 * @param position Desired position in milliseconds
 */
void VLCPlayerHandler::setPosition(qint64 position) {
    if (!m_mediaPlayer) return;

    // Calculate position percentage for DASH streaming
    libvlc_time_t duration = libvlc_media_player_get_length(m_mediaPlayer);
    float percentage = static_cast<float>(position) / duration;

    libvlc_media_player_set_position(m_mediaPlayer, percentage);

    qDebug() << "Seeking to position:" << position
        << "Duration:" << duration
        << "Percentage:" << percentage;
}

/**
 * @brief Starts media playback from specified position
 * @param percentage_watched Starting position as percentage (0.0 - 1.0)
 */
void VLCPlayerHandler::playMedia(float percentage_watched = 0) {
    if (m_mediaPlayer) {
        libvlc_media_player_play(m_mediaPlayer);

        // Wait for player to enter playing or paused state
        libvlc_state_t state = libvlc_media_player_get_state(m_mediaPlayer);
        while (!(state == libvlc_Playing || state == libvlc_Paused)) {
            state = libvlc_media_player_get_state(m_mediaPlayer);
        }

        m_isPlaying = true;

        // Set initial position if specified
        if (percentage_watched > 0.0) {
            libvlc_media_player_set_position(m_mediaPlayer, percentage_watched);
            libvlc_time_t currentTime = libvlc_media_player_get_time(m_mediaPlayer);
            emit positionChanged(currentTime);
        }

        // Start timers and notify state change
        m_positionTimer->start();
        m_metadataTimer->start();
        emit playingStateChanged(true);
    }
}

/**
 * @brief Pauses media playback
 */
void VLCPlayerHandler::pauseMedia() {
    if (m_mediaPlayer) {
        libvlc_media_player_pause(m_mediaPlayer);
        m_isPlaying = false;
        m_positionTimer->stop();
        emit playingStateChanged(false);
        emit positionChanged(libvlc_media_player_get_time(m_mediaPlayer));
    }
}

/**
 * @brief Stops media playback and closes video window
 */
void VLCPlayerHandler::stop() {
    if (m_mediaPlayer) {
        libvlc_media_player_stop(m_mediaPlayer);
        m_isPlaying = false;
        m_positionTimer->stop();

        // Clean up video window
        if (m_vlcWindow) {
            m_vlcWindow->close();
            delete m_vlcWindow;
            m_vlcWindow = nullptr;
        }

        emit playingStateChanged(false);
        emit positionChanged(0);
    }
}

/**
 * @brief Advances playback position by 30 seconds
 */
void VLCPlayerHandler::forward30sec() {
    if (m_mediaPlayer) {
        libvlc_time_t current_time = libvlc_media_player_get_time(m_mediaPlayer);
        libvlc_time_t new_time = current_time + 30000;
        libvlc_media_player_set_time(m_mediaPlayer, new_time);
        emit positionChanged(libvlc_media_player_get_time(m_mediaPlayer));
    }
}

/**
 * @brief Rewinds playback position by 30 seconds
 */
void VLCPlayerHandler::back30sec() {
    if (m_mediaPlayer) {
        libvlc_time_t current_time = libvlc_media_player_get_time(m_mediaPlayer);
        libvlc_time_t new_time = current_time - 30000;
        libvlc_media_player_set_time(m_mediaPlayer, new_time);
        emit positionChanged(libvlc_media_player_get_time(m_mediaPlayer));
    }
}

/**
 * @brief Updates media playback information and emits signals
 */
void VLCPlayerHandler::updateMediaInfo() {
    if (!m_mediaPlayer) return;

    libvlc_state_t state = libvlc_media_player_get_state(m_mediaPlayer);
    if (state == libvlc_Playing || state == libvlc_Paused) {
        libvlc_time_t currentTime = libvlc_media_player_get_time(m_mediaPlayer);
        libvlc_time_t duration = libvlc_media_player_get_length(m_mediaPlayer);
        float position = libvlc_media_player_get_position(m_mediaPlayer);

        if (currentTime >= 0 && duration > 0) {
            emit positionChanged(currentTime);
        }
    }
    else if (state == libvlc_Ended) {
        updateMediaMetadataOnServer();
        emit mediaEnded();
    }
}

/**
 * @brief Triggers an immediate metadata update
 */
void VLCPlayerHandler::updateMediaMetadata() {
    updateMediaMetadataOnServer();
}

/**
 * @brief Sends current media playback metadata to the server
 */
void VLCPlayerHandler::updateMediaMetadataOnServer() {
    if (!m_mediaPlayer) return;

    // Prepare network request
    QUrl url(m_url + "/update_media_metadata");
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("Authorization", "Bearer " + m_token.toUtf8());

    // Create metadata payload
    QJsonObject jsonPayload;
    jsonPayload["profileID"] = m_profileId;
    jsonPayload["mediaID"] = m_currentMediaId;

    // Calculate position based on playback state
    libvlc_state_t state = libvlc_media_player_get_state(m_mediaPlayer);
    float position = (state != libvlc_Ended) ?
        libvlc_media_player_get_position(m_mediaPlayer) : 1.0;

    jsonPayload["percentageWatched"] = QString::number(position, 'f', 3);
    jsonPayload["languageChosen"] = m_currentAudioText;
    jsonPayload["subtitlesChosen"] = m_currentSubtitlesText;

    // Send update request
    QJsonDocument doc(jsonPayload);
    QByteArray jsonData = doc.toJson();
    QNetworkReply* reply = m_networkManager.post(request, jsonData);

    // Handle response
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

/**
 * @brief Cleans up VLC resources
 */
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

/**
 * @brief Returns the total duration of the current media
 * @return qint64 Duration in milliseconds
 */
qint64 VLCPlayerHandler::duration() const {
    return m_mediaPlayer ? (qint64)libvlc_media_player_get_length(m_mediaPlayer) : 0;
}

/**
 * @brief Returns the current playback position
 * @return qint64 Position in milliseconds
 */
qint64 VLCPlayerHandler::position() const {
    return m_mediaPlayer ? (qint64)libvlc_media_player_get_time(m_mediaPlayer) : 0;
}

/**
 * @brief Returns the current playing state
 * @return bool True if media is playing
 */
bool VLCPlayerHandler::isPlaying() const {
    return m_isPlaying;
}

/**
 * @brief Sets the audio volume
 * @param volume Volume level (0-100)
 */
void VLCPlayerHandler::setVolume(int volume) {
    libvlc_audio_set_volume(m_mediaPlayer, volume);
}

/**
 * @brief Returns the current video sink
 * @return QVideoSink* Pointer to the video sink
 */
QVideoSink* VLCPlayerHandler::videoSink() const {
    return m_videoSink;
}

/**
 * @brief Sets up the video sink and attaches video output
 * @param sink Pointer to the QVideoSink to use
 */
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

    // Handle window visibility changes
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

/**
 * @brief Attaches video output to a QQuickItem
 * @param videoOutput The QQuickItem to attach to
 */
void VLCPlayerHandler::attachVideoOutput(QQuickItem* videoOutput) {
    if (!videoOutput || !videoOutput->window())
        return;

    // Create and configure VLC window
    m_vlcWindow = new QWindow(videoOutput->window());
    mainWindow = videoOutput->window();
    QRect mainRect = mainWindow->geometry();
    QPointF pos = mainWindow->position();

    // Calculate window dimensions
    int adjustedHeight = mainRect.height() - 160;
    if (adjustedHeight < 0) {
        qWarning() << "Adjusted height is negative, setting to minimum height of 10.";
        adjustedHeight = 10;
    }

    // Set up window properties
    m_vlcWindow->setGeometry(0, 0, mainRect.width(), adjustedHeight);
    m_vlcWindow->setFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
    m_vlcWindow->show();

    QCoreApplication::instance()->installEventFilter(this);

    // Platform-specific window handle setup
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

    libvlc_video_set_scale(m_mediaPlayer, 0);
}

/**
 * @brief Event filter for handling keyboard events in fullscreen mode
 * @param obj Object that triggered the event
 * @param event The event to process
 * @return bool True if event was handled, false otherwise
 */
bool VLCPlayerHandler::eventFilter(QObject* obj, QEvent* event) {
    if (fullScreen) {
        if (event->type() == QEvent::KeyPress) {
            QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);
            if (keyEvent->key() == Qt::Key_Escape) {
                setFullScreen(false);
                return true;
            }
        }
    }
    return QObject::eventFilter(obj, event);
}

/**
 * @brief Toggles fullscreen mode for the video window
 * @param setFullScreen True to enter fullscreen, false to exit
 */
void VLCPlayerHandler::setFullScreen(bool setFullScreen) {
    if (!m_vlcWindow)
        return;

    fullScreen = setFullScreen;
    QRect mainRect = mainWindow->geometry();

    if (fullScreen) {
        m_vlcWindow->setGeometry(0, 0, mainRect.width(), mainRect.height());
    }
    else {
        m_vlcWindow->setGeometry(0, 0, mainRect.width(), mainRect.height() - 160);
    }

    m_vlcWindow->show();
}

/**
 * @brief Loads and initializes media for playback
 * @param mediaId Unique identifier for the media
 * @param mediaMetadata Additional metadata for the media
 */
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

    // Clean up existing media
    if (m_media) {
        libvlc_media_release(m_media);
        m_media = nullptr;
    }

    // Create media URL and initialize
    QString baseUrl = QString(m_url + "/media/%1/manifest").arg(mediaId);
    QByteArray urlBytes = baseUrl.toUtf8();
    m_media = libvlc_media_new_location(m_vlcInstance, urlBytes.constData());

    if (m_media) {
        // Set media options
        libvlc_media_add_option(m_media, ":network-caching=1000");
        libvlc_media_add_option(m_media, ":http-reconnect");
        libvlc_media_add_option(m_media, ":adaptive-formats=dash");

        libvlc_media_player_set_media(m_mediaPlayer, m_media);

        // Initialize subtitles and audio
        tryDownloadSubtitles(mediaId);
        playMedia(percentage_watched);
        loadSubtitleTracks(mediaMetadata.value("subtitles_chosen").toString());
        loadAudioTracks(mediaMetadata.value("language_chosen").toString());

        // Emit signals for UI updates
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

/**
 * @brief Downloads and adds subtitle tracks for the media
 * @param mediaId ID of the media to download subtitles for
 * @return bool True if subtitles were successfully downloaded
 */
bool VLCPlayerHandler::tryDownloadSubtitles(const QString& mediaId) {
    QList<QString> languages;
    languages.append("es");
    languages.append("en");

    for (const QString& language : languages) {
        QUrl url(QString(m_url + "/media/%1/subtitles/%2").arg(mediaId, language + ".vtt"));

        // Verify subtitle availability
        QNetworkAccessManager manager;
        QNetworkRequest request(url);
        QEventLoop loop;
        QNetworkReply* reply = manager.get(request);
        QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
        loop.exec();

        int httpStatusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        if (reply->error() != QNetworkReply::NoError || httpStatusCode != 200) {
            qWarning() << "Subtitle URL does not exist or returned an error. Status code:" << httpStatusCode;
            reply->deleteLater();
            break;
        }
        reply->deleteLater();

        // Add subtitle track to media player
        if (m_mediaPlayer) {
            QByteArray urlBytes = url.toString().toUtf8();
            const char* charurl = urlBytes.constData();
            qDebug() << charurl;

            int result = libvlc_media_player_add_slave(
                m_mediaPlayer,
                libvlc_media_slave_type_subtitle,
                charurl,
                false
            );

            qDebug() << "Result of adding subtitle:" << result;
            if (result != 0) {
                qWarning() << "Failed to add subtitle track:" << language << "Error code:" << result;
            }
        }
        else {
            qWarning() << "Media player is not initialized.";
            return false;
        }
    }
    return true;
}

/**
 * @brief Loads and initializes subtitle tracks
 * @param subtitles_chosen Previously selected subtitle track
 */
void VLCPlayerHandler::loadSubtitleTracks(QString subtitles_chosen) {
    m_subtitleTracks.clear();

    if (!m_mediaPlayer) return;

    m_currentSubtitlesId = libvlc_video_get_spu(m_mediaPlayer);
    int metadataSubtitleId = m_currentSubtitlesId;

    // Get subtitle track descriptions
    libvlc_track_description_t* tracks = libvlc_video_get_spu_description(m_mediaPlayer);
    libvlc_track_description_t* currentTrack = tracks;

    // Process each subtitle track
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

    // Set subtitle track if different from current
    if (metadataSubtitleId != m_currentSubtitlesId) {
        setSubtitleTrack(metadataSubtitleId);
    }

    reorderListById(m_subtitleTracks, m_currentSubtitlesId);
    emit subtitleTracksChanged();
}

/**
 * @brief Sets the active subtitle track
 * @param trackId ID of the subtitle track to activate
 */
void VLCPlayerHandler::setSubtitleTrack(int trackId) {
    if (!m_mediaPlayer) return;

    libvlc_video_set_spu(m_mediaPlayer, trackId);
    qDebug() << "Setting subtitles track to:" << trackId;
    updateSubtitleSelected();
}

/**
 * @brief Disables subtitles
 */
void VLCPlayerHandler::disableSubtitles() {
    if (m_mediaPlayer) {
        libvlc_video_set_spu(m_mediaPlayer, -1);
    }
}

/**
 * @brief Returns the list of available subtitle tracks
 * @return QVariantList List of subtitle track information
 */
QVariantList VLCPlayerHandler::subtitleTracks() const {
    return m_subtitleTracks;
}

/**
 * @brief Returns the list of available audio tracks
 * @return QVariantList List of audio track information
 */
QVariantList VLCPlayerHandler::audioTracks() const {
    return m_audioTracks;
}

/**
 * @brief Reorders a track list to put selected track first
 * @param list List to reorder
 * @param selectedTrackId ID of the track to move to front
 */
void VLCPlayerHandler::reorderListById(QVariantList& list, int selectedTrackId) {
    int targetIndex = -1;
    for (int i = 0; i < list.size(); ++i) {
        QVariantMap map = list[i].toMap();
        if (map["id"].toInt() == selectedTrackId) {
            targetIndex = i;
            break;
        }
    }

    if (targetIndex != -1) {
        QVariant targetMap = list.takeAt(targetIndex);
        list.prepend(targetMap);
    }
}

/**
 * @brief Updates the currently selected audio track information
 */
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

/**
 * @brief Updates the currently selected subtitle track information
 */
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

/**
 * @brief Loads and initializes audio tracks
 * @param languageChosen Previously selected audio language
 */
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

/**
 * @brief Sets the active audio track
 * @param trackId ID of the audio track to activate
 */
void VLCPlayerHandler::setAudioTrack(int trackId) {
    if (!m_mediaPlayer) return;

    libvlc_audio_set_track(m_mediaPlayer, trackId);
    qDebug() << "Setting audio track to:" << trackId;
    updateAudioSelected();
}

/**
 * @brief Returns the index of the current audio track
 * @return int Current audio track index
 */
int VLCPlayerHandler::getAudioIndex() {
    return m_currentAudioId;
}

/**
 * @brief Returns the name/description of the current audio track
 * @return QString Name of the current audio track
 */
QString VLCPlayerHandler::getAudioText() {
    return m_currentAudioText;
}


