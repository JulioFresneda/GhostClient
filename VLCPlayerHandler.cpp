#include "VLCPlayerHandler.h"
#include <stdexcept>

VLCPlayerHandler::VLCPlayerHandler(QObject* parent)
    : QObject(parent)
    , vlcInstance(libvlc_new(0, nullptr), libvlc_release)
    , mediaPlayer(nullptr, libvlc_media_player_release)
    , media(nullptr)
    , positionTimer(new QTimer(this))
{
    if (!vlcInstance) {
        throw std::runtime_error("Failed to create VLC instance");
    }

    connect(positionTimer, &QTimer::timeout, this, &VLCPlayerHandler::mediaStateChanged);
}

VLCPlayerHandler::~VLCPlayerHandler()
{
    releaseMedia();
}

void VLCPlayerHandler::setMedia(const QString& mediaUrl)
{
    releaseMedia();
    media = libvlc_media_new_location(vlcInstance.get(), mediaUrl.toUtf8().constData());

    if (!media) {
        throw std::runtime_error("Failed to set media");
    }

    mediaPlayer.reset(libvlc_media_player_new_from_media(media));
    if (!mediaPlayer) {
        throw std::runtime_error("Failed to create VLC media player");
    }
}

void VLCPlayerHandler::play()
{
    if (mediaPlayer) {
        libvlc_media_player_play(mediaPlayer.get());
        positionTimer->start(1000); // Update position every second
        emit mediaStateChanged();
    }
}

void VLCPlayerHandler::pause()
{
    if (mediaPlayer) {
        libvlc_media_player_pause(mediaPlayer.get());
        positionTimer->stop();
        emit mediaStateChanged();
    }
}

void VLCPlayerHandler::stop()
{
    if (mediaPlayer) {
        libvlc_media_player_stop(mediaPlayer.get());
        positionTimer->stop();
        emit mediaStateChanged();
    }
}

void VLCPlayerHandler::setSubtitleTrack(int trackId)
{
    if (mediaPlayer) {
        libvlc_video_set_spu(mediaPlayer.get(), trackId);
    }
}

void VLCPlayerHandler::disableSubtitles()
{
    if (mediaPlayer) {
        libvlc_video_set_spu(mediaPlayer.get(), -1);
    }
}

void VLCPlayerHandler::releaseMedia()
{
    if (media) {
        libvlc_media_release(media);
        media = nullptr;
    }
}