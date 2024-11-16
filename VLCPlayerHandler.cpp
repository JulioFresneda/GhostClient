#include "VLCPlayerHandler.h"
#include <QCoreApplication>
#include <QDir>
#include <QSettings>
#include <QUrl>
#include <QUrlQuery>
#include <QDebug>


#include <QQuickItem>
#include <QQuickWindow>

#ifdef Q_OS_WIN
#include <windows.h>
#endif

VLCPlayerHandler::VLCPlayerHandler(QObject* parent)
    : QObject(parent)
    , m_vlcInstance(nullptr)
    , m_mediaPlayer(nullptr)
    , m_media(nullptr)
    , m_isPlaying(false)
    , m_videoSink(nullptr)
{
    QSettings settings("./conf.ini", QSettings::IniFormat);
    m_token = settings.value("authToken").toString();
    m_userId = settings.value("userID").toString();

    // Initialize VLC with specific video output options
    const char* args[] = {
        "--no-video-title-show",      // Don't show video title
        //"--no-xlib",      // Disable direct rendering
        "--clock-jitter=0",// Reduce VLC's output,
        "--no-mouse-events"
    };

    m_vlcInstance = libvlc_new(sizeof(args) / sizeof(*args), args);

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
    m_positionTimer->setInterval(1000);
}

VLCPlayerHandler::~VLCPlayerHandler() {
    cleanupVLC();
}

void VLCPlayerHandler::initializeVLC() {
    QString appDir = QCoreApplication::applicationDirPath();
    QString pluginPath = appDir + "/plugins";
    qDebug() << "Initializing VLC with plugin path:" << pluginPath;

    // Store the plugin path string as a QByteArray to keep it in scope
    QByteArray pluginPathArg = QString("--plugin-path=" + pluginPath).toUtf8();

    qDebug() << "Plugin path:" << pluginPathArg.constData();


    // Create an array of const char* for all VLC arguments
    const char* args[] = {
        pluginPathArg.constData(),    // Plugin path
        "--verbose=2",                // Verbose logging
        "--network-caching=1000",     // Network cache value
        "--http-reconnect",           // Enable HTTP reconnection
        "--http-forward-cookies",     // Enable cookie forwarding
        //"--http-trust-headers"        // Trust custom headers
    };

    // Initialize VLC with all arguments
    int sizetotal = sizeof(args) / sizeof(args[0]);
    libvlc_set_log_verbosity(m_vlcInstance, true);
    m_vlcInstance = libvlc_new(sizetotal, args);
    

    const char* error = libvlc_errmsg();
    if (error) {
        printf("libvlc_new() error: %s\n", error);
    }

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

    qDebug() << "VLC initialized successfully";
}

void VLCPlayerHandler::loadMedia(const QString& mediaId) {
    if (!verifyVLCSetup()) {
        return;
    }

    if (m_media) {
        libvlc_media_release(m_media);
        m_media = nullptr;
    }

    QString baseUrl = QString("http://localhost:18080/media/%1/manifest?userID=%2&token=%3")
        .arg(mediaId)
        .arg(m_userId)
        .arg(m_token);

    QByteArray urlBytes = baseUrl.toUtf8();
    m_media = libvlc_media_new_location(m_vlcInstance, urlBytes.constData());

    if (m_media) {
        libvlc_media_add_option(m_media, ":network-caching=1000");
        libvlc_media_add_option(m_media, ":http-reconnect");
        libvlc_media_add_option(m_media, ":demux=adaptive");
        libvlc_media_add_option(m_media, ":adaptive-formats=dash");

        libvlc_media_player_set_media(m_mediaPlayer, m_media);
        emit mediaLoaded();

        if (m_isPlaying) {
            playMedia();
        }
    }
    else {
        qDebug() << "Failed to create media";
        emit errorOccurred("Failed to create media");
    }
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

QString VLCPlayerHandler::constructManifestUrl(const QString& mediaId) {
    QUrl url(QString("http://localhost:18080/media/%1/manifest").arg(mediaId));
    QUrlQuery query;
    query.addQueryItem("token", m_token);
    query.addQueryItem("userID", m_userId);
    url.setQuery(query);
    return url.toString();
}

void VLCPlayerHandler::playMedia() {
    if (m_mediaPlayer) {
        libvlc_media_player_play(m_mediaPlayer);
        m_isPlaying = true;
        m_positionTimer->start();
        emit playingStateChanged(true);
    }
}

void VLCPlayerHandler::pauseMedia() {
    if (m_mediaPlayer) {
        libvlc_media_player_pause(m_mediaPlayer);
        m_isPlaying = false;
        m_positionTimer->stop();
        emit playingStateChanged(false);
    }
}

void VLCPlayerHandler::stop() {
    if (m_mediaPlayer) {
        libvlc_media_player_stop(m_mediaPlayer);
        m_isPlaying = false;
        m_positionTimer->stop();
        emit playingStateChanged(false);
    }
}

void VLCPlayerHandler::updateMediaInfo() {
    if (m_mediaPlayer) {
        emit positionChanged(libvlc_media_player_get_time(m_mediaPlayer));
        emit durationChanged(libvlc_media_player_get_length(m_mediaPlayer));
    }
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

void VLCPlayerHandler::setPosition(qint64 position) {
    if (m_mediaPlayer) {
        libvlc_media_player_set_time(m_mediaPlayer, position);  // Removed the third argument
    }
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

    // Get the QQuickItem that owns the VideoOutput
    auto videoOutput = m_videoSink->parent();
    if (!videoOutput || !videoOutput->inherits("QQuickItem"))
        return;

    QQuickItem* quickVideoOutput = qobject_cast<QQuickItem*>(videoOutput);
    if (!quickVideoOutput || !quickVideoOutput->window())
        return;

    // Wait for the window to be ready
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

void VLCPlayerHandler::attachVideoOutput(QQuickItem* videoOutput) {
    if (!videoOutput || !videoOutput->window())
        return;

    QWindow* window = videoOutput->window();

#ifdef Q_OS_WIN
    WId handle = window->winId();
    if (handle) {
        // On Windows, we use the HWND
        libvlc_media_player_set_hwnd(m_mediaPlayer, (void*)handle);
    }
#elif defined(Q_OS_LINUX)
    // On Linux, we use the X11 window
    libvlc_media_player_set_xwindow(m_mediaPlayer, window->winId());
#elif defined(Q_OS_MAC)
    // On macOS, we use the NSView
    libvlc_media_player_set_nsobject(m_mediaPlayer, (void*)window->winId());
#endif

    // Store the video output dimensions
    QRectF rect = videoOutput->mapRectToScene(videoOutput->boundingRect());
    libvlc_video_set_scale(m_mediaPlayer, 0); // Auto scale
}