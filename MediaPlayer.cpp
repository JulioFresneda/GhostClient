#include "MediaPlayer.h"
#include <QUrl>
#include <QVideoSink>
#include <QVideoFrame>
#include <QSettings>
#include <QDebug>

MediaPlayer::MediaPlayer(QQuickItem* parent)
    : QQuickItem(parent)
    , m_isInitialized(false)
    , m_videoSink(nullptr)
    , m_isPlaying(false)
{
    setFlag(ItemHasContents, true);
    connect(&m_player, &VLCPlayerHandler::mediaStateChanged,
        this, &MediaPlayer::handleMediaStateChanged);

    // Load stored settings
    QSettings settings("./conf.ini", QSettings::IniFormat);
    m_storedToken = settings.value("authToken", "").toString();
    m_userID = settings.value("userID", "").toString();
    m_selectedProfileID = settings.value("selectedProfileID", "").toString();
}

void MediaPlayer::setMediaId(const QString& mediaId)
{
    if (m_mediaId != mediaId) {
        m_mediaId = mediaId;
        emit mediaIdChanged();

        if (!mediaId.isEmpty()) {
            initializePlayer();
        }
    }
}

void MediaPlayer::setTitle(const QString& title)
{
    if (m_title != title) {
        m_title = title;
        emit titleChanged();
    }
}

void MediaPlayer::setVideoSink(QVideoSink* sink)
{
    if (m_videoSink != sink) {
        m_videoSink = sink;

        if (m_isInitialized) {
            // Update VLC player output to use the new sink
            try {
                // TODO: Implement video sink connection to VLC
                // This would involve setting up a callback from VLC to push frames to the sink
                // m_player.setVideoOutput(sink);
            }
            catch (const std::exception& e) {
                emit error(QString("Failed to set video output: %1").arg(e.what()));
            }
        }
        emit videoSinkChanged();
    }
}

void MediaPlayer::play()
{
    try {
        if (!m_isInitialized) {
            initializePlayer();
        }
        m_player.play();
        m_isPlaying = true;
        emit isPlayingChanged();
    }
    catch (const std::exception& e) {
        emit error(QString("Failed to play media: %1").arg(e.what()));
    }
}

void MediaPlayer::pause()
{
    try {
        m_player.pause();
        m_isPlaying = false;
        emit isPlayingChanged();
    }
    catch (const std::exception& e) {
        emit error(QString("Failed to pause media: %1").arg(e.what()));
    }
}

void MediaPlayer::stop()
{
    try {
        m_player.stop();
        m_isPlaying = false;
        emit isPlayingChanged();
    }
    catch (const std::exception& e) {
        emit error(QString("Failed to stop media: %1").arg(e.what()));
    }
}

void MediaPlayer::seek(qint64 position)
{
    try {
        // TODO: Implement seeking through VLC
        // m_player.setPosition(position);
        emit positionChanged();
    }
    catch (const std::exception& e) {
        emit error(QString("Failed to seek: %1").arg(e.what()));
    }
}

void MediaPlayer::setVolume(int volume)
{
    try {
        // TODO: Implement volume control through VLC
        // m_player.setVolume(volume);
        emit volumeChanged();
    }
    catch (const std::exception& e) {
        emit error(QString("Failed to set volume: %1").arg(e.what()));
    }
}

void MediaPlayer::setSubtitles(int trackId)
{
    try {
        m_player.setSubtitleTrack(trackId);
    }
    catch (const std::exception& e) {
        emit error(QString("Failed to set subtitles: %1").arg(e.what()));
    }
}

void MediaPlayer::disableSubtitles()
{
    try {
        m_player.disableSubtitles();
    }
    catch (const std::exception& e) {
        emit error(QString("Failed to disable subtitles: %1").arg(e.what()));
    }
}

void MediaPlayer::closePlayer()
{
    stop();
    m_isInitialized = false;
    emit closeRequested();
}

void MediaPlayer::handleMediaStateChanged()
{
    try {
        // TODO: Handle media state changes from VLC
        // Could include things like:
        // - Updating duration
        // - Handling end of media
        // - Handling errors
        // - Updating current position
        emit mediaStateChanged();
    }
    catch (const std::exception& e) {
        emit error(QString("Media state error: %1").arg(e.what()));
    }
}

void MediaPlayer::initializePlayer()
{
    if (m_mediaId.isEmpty()) {
        emit error("Cannot initialize player: No media ID provided");
        return;
    }

    try {
        // Construct the manifest URL with authentication parameters
        QString manifestUrl = QString("http://localhost:18080/media/%1/manifest?userID=%2&token=%3")
            .arg(m_mediaId)
            .arg(m_userID)
            .arg(m_storedToken);

        qDebug() << "Initializing player with URL:" << manifestUrl;

        // Set the media URL in the VLC player
        m_player.setMedia(manifestUrl);
        m_isInitialized = true;

        // Set up video output if sink is available
        if (m_videoSink) {
            // TODO: Set up video output
            // m_player.setVideoOutput(m_videoSink);
        }

        emit durationChanged(); // In case duration is available immediately
    }
    catch (const std::exception& e) {
        m_isInitialized = false;
        emit error(QString("Failed to initialize player: %1").arg(e.what()));
    }
}

bool MediaPlayer::isPlaying() const
{
    return m_isPlaying;
}

qint64 MediaPlayer::position() const
{
    // TODO: Implement position getting from VLC
    return 0;
}

qint64 MediaPlayer::duration() const
{
    // TODO: Implement duration getting from VLC
    return 0;
}

int MediaPlayer::volume() const
{
    // TODO: Implement volume getting from VLC
    return 100;
}