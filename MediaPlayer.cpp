#include "MediaPlayer.h"

MediaPlayer::MediaPlayer(QObject* parent)
    : QObject(parent)
    , m_vlcPlayer(new VLCPlayerHandler(this))
{
    // Connect VLC player signals
    connect(m_vlcPlayer, &VLCPlayerHandler::playingStateChanged,
        this, &MediaPlayer::isPlayingChanged);
    connect(m_vlcPlayer, &VLCPlayerHandler::positionChanged,
        this, &MediaPlayer::positionChanged);
    connect(m_vlcPlayer, &VLCPlayerHandler::durationChanged,
        this, &MediaPlayer::durationChanged);

}

void MediaPlayer::setMediaId(const QString& mediaId) {
    if (m_mediaId != mediaId) {
        m_mediaId = mediaId;
        if (!mediaId.isEmpty()) {
            //m_vlcPlayer->loadMedia(mediaId);
        }
        emit mediaIdChanged();
    }
}

void MediaPlayer::setTitle(const QString& title) {
    if (m_title != title) {
        m_title = title;
        emit titleChanged();
    }
}

bool MediaPlayer::isPlaying() const {
    return m_vlcPlayer->isPlaying();
}


qint64 MediaPlayer::position() const {
    return m_vlcPlayer->position();
}

qint64 MediaPlayer::duration() const {
    return m_vlcPlayer->duration();
}

void MediaPlayer::setPosition(qint64 position) {
    m_vlcPlayer->setPosition(position);
}

void MediaPlayer::play() {
    m_vlcPlayer->playMedia(0);
}

void MediaPlayer::pause() {
    m_vlcPlayer->pauseMedia();
}

void MediaPlayer::stop() {
    m_vlcPlayer->stop();
}

void MediaPlayer::forward30sec() {
    m_vlcPlayer->forward30sec();
}

void MediaPlayer::back30sec() {
    m_vlcPlayer->back30sec();
}

void MediaPlayer::close() {
    stop();
    emit closeRequested();
}

