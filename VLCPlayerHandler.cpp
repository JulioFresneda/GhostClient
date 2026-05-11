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
#include <QVideoFrame>
#include <QVideoFrameFormat>
#include <QMutexLocker>
#include <QElapsedTimer>
#include <QThread>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <algorithm>
#ifdef Q_OS_WIN
#include <windows.h>
#else
#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusReply>
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
    // libVLC levels: 0=DEBUG, 2=NOTICE, 3=WARNING, 4=ERROR.
    // Skip everything below WARNING so our [GHOST] lines aren't lost in noise.
    if (level < 3) return;
    char buf[1024];
    vsnprintf(buf, sizeof(buf), fmt, args);
    fprintf(stderr, "[VLC log level=%d] %s\n", level, buf);
    fflush(stderr);
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
    , last_percentage_watched(0.0)
    , fullScreen(false)
    , m_videoWidth(0)
    , m_videoHeight(0)
    , m_planeY(nullptr)
    , m_planeU(nullptr)
    , m_planeV(nullptr)
    , m_pitchY(0)
    , m_pitchU(0)
    , m_pitchV(0)
    , m_linesY(0)
    , m_linesU(0)
    , m_linesV(0)
    , m_frameDeliveryPending(false)
{
    // Initialize configuration from settings file
#ifdef PROJECT_ROOT_DIR
    QString configPath = QString(PROJECT_ROOT_DIR) + "/conf.ini";
#else
    QString configPath = QCoreApplication::applicationDirPath() + "/conf.ini";
#endif
    QSettings settings(configPath, QSettings::IniFormat);
    m_token = settings.value("token").toString();
    m_profileId = settings.value("selectedProfileID").toString();
    QString port = settings.value("port").toString();
    bool isLocalhost = settings.value("localhost", "false").toBool();
    QString host = isLocalhost ? "localhost" : settings.value("domain").toString();
    QString scheme = isLocalhost ? "http" : "https";
    m_url = scheme + "://" + host + ":" + port;

    fprintf(stderr, "[GHOST] VLCPlayerHandler constructor\n");
    fprintf(stderr, "[GHOST] conf.ini path resolved to: %s\n", QFileInfo(configPath).absoluteFilePath().toUtf8().constData());
    fprintf(stderr, "[GHOST] m_url = %s\n", m_url.toUtf8().constData());
    fprintf(stderr, "[GHOST] token present: %s\n", m_token.isEmpty() ? "NO" : "YES");
    fflush(stderr);

    // VLC command line arguments
    const char* args[] = {
        "--quiet",
    };

    // Initialize VLC instance
    fprintf(stderr, "[GHOST] Calling libvlc_new...\n"); fflush(stderr);
    m_vlcInstance = libvlc_new(sizeof(args) / sizeof(*args), args);
    fprintf(stderr, "[GHOST] libvlc_new returned: %p\n", (void*)m_vlcInstance); fflush(stderr);
    if (!m_vlcInstance) {
        fprintf(stderr, "[GHOST] ERROR: Failed to create VLC instance\n"); fflush(stderr);
        return;
    }
    libvlc_set_log_verbosity(m_vlcInstance, 2);

    // Create media player instance
    fprintf(stderr, "[GHOST] Calling libvlc_media_player_new...\n"); fflush(stderr);
    m_mediaPlayer = libvlc_media_player_new(m_vlcInstance);
    fprintf(stderr, "[GHOST] libvlc_media_player_new returned: %p\n", (void*)m_mediaPlayer); fflush(stderr);
    if (!m_mediaPlayer) {
        fprintf(stderr, "[GHOST] ERROR: Failed to create media player\n"); fflush(stderr);
        libvlc_release(m_vlcInstance);
        m_vlcInstance = nullptr;
        return;
    }

    // Route decoded frames into our QVideoSink instead of a native window.
    // Must be set before play; safe to set once for the player's lifetime.
    libvlc_video_set_format_callbacks(m_mediaPlayer,
                                      &VLCPlayerHandler::videoFormatCallback,
                                      &VLCPlayerHandler::videoFormatCleanupCallback);
    libvlc_video_set_callbacks(m_mediaPlayer,
                               &VLCPlayerHandler::videoLockCallback,
                               &VLCPlayerHandler::videoUnlockCallback,
                               &VLCPlayerHandler::videoDisplayCallback,
                               this);

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
    uninhibitIdle();
    cleanupVLC();
}

void VLCPlayerHandler::inhibitIdle() {
    if (m_inhibitActive) return;
#ifdef Q_OS_WIN
    // ES_SYSTEM_REQUIRED prevents the idle-to-sleep timer; ES_DISPLAY_REQUIRED
    // keeps the screen on. ES_CONTINUOUS makes it persist until we clear it.
    SetThreadExecutionState(ES_CONTINUOUS | ES_SYSTEM_REQUIRED | ES_DISPLAY_REQUIRED);
    m_inhibitActive = true;
#else
    QDBusInterface ss("org.freedesktop.ScreenSaver",
                      "/org/freedesktop/ScreenSaver",
                      "org.freedesktop.ScreenSaver",
                      QDBusConnection::sessionBus());
    if (!ss.isValid()) {
        fprintf(stderr, "[GHOST] inhibitIdle: ScreenSaver D-Bus iface not available\n");
        fflush(stderr);
        return;
    }
    QDBusReply<quint32> reply = ss.call("Inhibit",
                                        QStringLiteral("GhostClient"),
                                        QStringLiteral("Playing media"));
    if (!reply.isValid()) {
        fprintf(stderr, "[GHOST] inhibitIdle: Inhibit call failed: %s\n",
                reply.error().message().toUtf8().constData());
        fflush(stderr);
        return;
    }
    m_inhibitCookie = reply.value();
    m_inhibitActive = true;
#endif
}

void VLCPlayerHandler::uninhibitIdle() {
    if (!m_inhibitActive) return;
#ifdef Q_OS_WIN
    SetThreadExecutionState(ES_CONTINUOUS);
#else
    QDBusInterface ss("org.freedesktop.ScreenSaver",
                      "/org/freedesktop/ScreenSaver",
                      "org.freedesktop.ScreenSaver",
                      QDBusConnection::sessionBus());
    if (ss.isValid() && m_inhibitCookie != 0) {
        ss.call("UnInhibit", m_inhibitCookie);
    }
    m_inhibitCookie = 0;
#endif
    m_inhibitActive = false;
}

/**
 * @brief Verifies that VLC components are properly initialized
 * @return bool True if setup is valid, false otherwise
 */
bool VLCPlayerHandler::verifyVLCSetup() {
    fprintf(stderr, "[GHOST] verifyVLCSetup: instance=%p player=%p\n", (void*)m_vlcInstance, (void*)m_mediaPlayer);
    fflush(stderr);
    if (!m_vlcInstance || !m_mediaPlayer) {
        QString error = "VLC setup verification failed: ";
        if (!m_vlcInstance) error += "VLC instance is null. ";
        if (!m_mediaPlayer) error += "Media player is null. ";

        fprintf(stderr, "[GHOST] ERROR: %s\n", error.toUtf8().constData()); fflush(stderr);
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

        // Wait for player to reach Playing/Paused, but bound it. Without the
        // timeout an error during open (bad URL, decoder crash) would pin a
        // CPU core and freeze the GUI forever. Yields between polls instead
        // of spinning.
        QElapsedTimer waitTimer;
        waitTimer.start();
        constexpr qint64 kStartTimeoutMs = 5000;
        libvlc_state_t state = libvlc_media_player_get_state(m_mediaPlayer);
        while (state != libvlc_Playing && state != libvlc_Paused) {
            if (state == libvlc_Error) {
                emit errorOccurred("VLC entered error state while starting playback");
                return;
            }
            if (waitTimer.elapsed() > kStartTimeoutMs) {
                emit errorOccurred("Timed out waiting for VLC to start playback");
                return;
            }
            QThread::msleep(10);
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
        inhibitIdle();
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
        uninhibitIdle();
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
        uninhibitIdle();
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
        m_media = nullptr;
    }
    if (m_mediaPlayer) {
        libvlc_media_player_release(m_mediaPlayer);
        m_mediaPlayer = nullptr;
    }
    if (m_vlcInstance) {
        libvlc_release(m_vlcInstance);
        m_vlcInstance = nullptr;
    }
    // VLC's format-cleanup callback normally frees these, but free defensively
    // in case the player is destroyed without ever decoding a frame.
    std::free(m_planeY);
    std::free(m_planeU);
    std::free(m_planeV);
    m_planeY = m_planeU = m_planeV = nullptr;
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
 * @brief Stores the QVideoSink that the QML VideoOutput is bound to.
 *
 * Decoded frames are pushed into this sink from the deliverFrame() slot,
 * which is invoked on the GUI thread via the videoDisplayCallback().
 */
void VLCPlayerHandler::setVideoSink(QVideoSink* sink) {
    if (m_videoSink == sink)
        return;
    m_videoSink = sink;
    emit videoSinkChanged();
}

/**
 * @brief Toggles fullscreen mode by signalling the QML layer.
 *
 * The actual layout change (hiding the controls strip and letting the
 * video item fill the window) is handled in MediaPlayer.qml.
 */
void VLCPlayerHandler::setFullScreen(bool setFullScreen) {
    if (fullScreen == setFullScreen)
        return;
    fullScreen = setFullScreen;
    emit fullScreenChanged(fullScreen);
}

// ---------------------------------------------------------------------------
// libVLC video callbacks
//
// VLC invokes these on its video output thread. We use software rendering
// into a single BGRA32 buffer and let m_frameMutex serialise access between
// VLC (lock/unlock) and the GUI thread (deliverFrame). Hardware decoding is
// implicitly disabled while these callbacks are installed.
// ---------------------------------------------------------------------------

unsigned VLCPlayerHandler::videoFormatCallback(void** opaque, char* chroma,
                                               unsigned* width, unsigned* height,
                                               unsigned* pitches, unsigned* lines) {
    auto* self = static_cast<VLCPlayerHandler*>(*opaque);

    // I420 = 8-bit YUV 4:2:0 planar. This is the decoder's native output for
    // most codecs, so VLC writes directly to our buffers without an internal
    // colour conversion pass — which is what was producing the green stripes
    // when we asked for RV32. Qt's video sink converts YUV→RGB on the GPU.
    std::memcpy(chroma, "I420", 4);

    const int w = static_cast<int>(*width);
    const int h = static_cast<int>(*height);
    // Round Y dimensions up to multiples of 32 so VLC's pipeline can write
    // its padded output safely. UV dimensions are half of the (aligned) Y.
    const int alignedW = (w + 31) & ~31;
    const int alignedH = (h + 31) & ~31;

    QMutexLocker lock(&self->m_frameMutex);
    self->m_videoWidth  = w;
    self->m_videoHeight = h;
    self->m_pitchY = alignedW;
    self->m_pitchU = alignedW / 2;
    self->m_pitchV = alignedW / 2;
    self->m_linesY = alignedH;
    self->m_linesU = alignedH / 2;
    self->m_linesV = alignedH / 2;

    pitches[0] = static_cast<unsigned>(self->m_pitchY);
    pitches[1] = static_cast<unsigned>(self->m_pitchU);
    pitches[2] = static_cast<unsigned>(self->m_pitchV);
    lines[0]   = static_cast<unsigned>(self->m_linesY);
    lines[1]   = static_cast<unsigned>(self->m_linesU);
    lines[2]   = static_cast<unsigned>(self->m_linesV);

    std::free(self->m_planeY);
    std::free(self->m_planeU);
    std::free(self->m_planeV);
    self->m_planeY = static_cast<uchar*>(std::calloc(self->m_pitchY, self->m_linesY));
    self->m_planeU = static_cast<uchar*>(std::calloc(self->m_pitchU, self->m_linesU));
    self->m_planeV = static_cast<uchar*>(std::calloc(self->m_pitchV, self->m_linesV));

    return 1;
}

void VLCPlayerHandler::videoFormatCleanupCallback(void* opaque) {
    auto* self = static_cast<VLCPlayerHandler*>(opaque);
    QMutexLocker lock(&self->m_frameMutex);
    std::free(self->m_planeY);
    std::free(self->m_planeU);
    std::free(self->m_planeV);
    self->m_planeY = self->m_planeU = self->m_planeV = nullptr;
    self->m_videoWidth = 0;
    self->m_videoHeight = 0;
    self->m_pitchY = self->m_pitchU = self->m_pitchV = 0;
    self->m_linesY = self->m_linesU = self->m_linesV = 0;
}

void* VLCPlayerHandler::videoLockCallback(void* opaque, void** planes) {
    auto* self = static_cast<VLCPlayerHandler*>(opaque);
    self->m_frameMutex.lock();
    planes[0] = self->m_planeY;
    planes[1] = self->m_planeU;
    planes[2] = self->m_planeV;
    return nullptr;
}

void VLCPlayerHandler::videoUnlockCallback(void* opaque, void* /*picture*/, void* const* /*planes*/) {
    auto* self = static_cast<VLCPlayerHandler*>(opaque);
    self->m_frameMutex.unlock();
}

void VLCPlayerHandler::videoDisplayCallback(void* opaque, void* /*picture*/) {
    auto* self = static_cast<VLCPlayerHandler*>(opaque);

    // Coalesce: if a delivery is already queued, drop this one — we'll just
    // pick up the newest frame the next time the GUI thread runs.
    {
        QMutexLocker lock(&self->m_frameMutex);
        if (self->m_frameDeliveryPending) return;
        self->m_frameDeliveryPending = true;
    }
    QMetaObject::invokeMethod(self, "deliverFrame", Qt::QueuedConnection);
}

/**
 * @brief GUI-thread slot that copies the latest VLC frame into a
 *        QVideoFrame and pushes it to the bound QVideoSink.
 */
void VLCPlayerHandler::deliverFrame() {
    QMutexLocker lock(&m_frameMutex);
    m_frameDeliveryPending = false;

    if (!m_videoSink || !m_planeY || !m_planeU || !m_planeV ||
        m_videoWidth <= 0 || m_videoHeight <= 0)
        return;

    // CPU YUV420 → BGRA (BT.601 limited range).
    QVideoFrameFormat format(QSize(m_videoWidth, m_videoHeight),
                             QVideoFrameFormat::Format_BGRA8888);
    QVideoFrame frame(format);
    if (!frame.map(QVideoFrame::WriteOnly))
        return;

    const int dstPitch = frame.bytesPerLine(0);
    uchar* dst = frame.bits(0);

    for (int y = 0; y < m_videoHeight; ++y) {
        const uchar* yRow = m_planeY + y * m_pitchY;
        const uchar* uRow = m_planeU + (y / 2) * m_pitchU;
        const uchar* vRow = m_planeV + (y / 2) * m_pitchV;
        uchar* dstRow = dst + y * dstPitch;

        for (int x = 0; x < m_videoWidth; ++x) {
            const int Y = yRow[x];
            const int U = uRow[x / 2] - 128;
            const int V = vRow[x / 2] - 128;

            int r = (Y << 10) + 1436 * V;
            int g = (Y << 10) - 352 * U - 731 * V;
            int b = (Y << 10) + 1814 * U;

            r = std::clamp(r >> 10, 0, 255);
            g = std::clamp(g >> 10, 0, 255);
            b = std::clamp(b >> 10, 0, 255);

            uchar* px = dstRow + x * 4;
            px[0] = static_cast<uchar>(b);
            px[1] = static_cast<uchar>(g);
            px[2] = static_cast<uchar>(r);
            px[3] = 0xFF;
        }
    }

    frame.unmap();
    m_videoSink->setVideoFrame(frame);
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
    QString baseUrl = QString(m_url + "/stream/%1").arg(mediaId);
    fprintf(stderr, "[GHOST] loadMedia URL: %s\n", baseUrl.toUtf8().constData()); fflush(stderr);
    QByteArray urlBytes = baseUrl.toUtf8();
    m_media = libvlc_media_new_location(m_vlcInstance, urlBytes.constData());
    fprintf(stderr, "[GHOST] libvlc_media_new_location returned: %p\n", (void*)m_media); fflush(stderr);

    if (m_media) {
        // Set media options
        libvlc_media_add_option(m_media, ":network-caching=5000");
        libvlc_media_add_option(m_media, ":http-reconnect");
        // Force pure software decode. With video callbacks libVLC has to copy
        // any HW-decoded surface back to CPU memory, and the VAOP→I420 chroma
        // conversion truncates the bottom chroma rows on this stream — that's
        // what produced the alternating green stripes at the bottom.
        libvlc_media_add_option(m_media, ":avcodec-hw=none");
        QByteArray authHeader = QString(":http-extra-headers=Authorization: Bearer %1\r\n").arg(m_token).toUtf8();
        libvlc_media_add_option(m_media, authHeader.constData());

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
        bool exist = true;
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
            exist = false;
        }
        if (exist) {
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

