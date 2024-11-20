#include "VLCPlayerHandler.h"
#include <stdexcept>

VLCPlayerHandler::VLCPlayerHandler(QObject* parent)
    : QObject(parent)
    , vlcInstance(libvlc_new(0, nullptr), libvlc_release)
    , mediaPlayer(nullptr, libvlc_media_player_release)
    , media(nullptr)
    , positionTimer(new QTimer(this))
{
    QSettings settings("./conf.ini", QSettings::IniFormat);
    m_token = settings.value("authToken").toString();
    m_userId = settings.value("userID").toString();

    const char* args[] = {
        //"--no-video-title-show",
        //"--clock-jitter=0",
        //"--no-mouse-events",
        //"--input-fast-seek",
        //"--network-caching=1000",
        //"--adaptive-maxwidth=1920",
        "--verbose=3",
        //"--file-logging",             // Enable file logging
        //"--logfile=C:\\Users\\julio\\Documents\\vlc-log.txt"
    };

    m_vlcInstance = libvlc_new(sizeof(args) / sizeof(*args), args);
    libvlc_set_log_verbosity(m_vlcInstance, 2);
    if (!m_vlcInstance) {
        qDebug() << "Failed to create VLC instance";
        return;
    }

    connect(positionTimer, &QTimer::timeout, this, &VLCPlayerHandler::mediaStateChanged);
}

    m_positionTimer = new QTimer(this);
    connect(m_positionTimer, &QTimer::timeout, this, &VLCPlayerHandler::updateMediaInfo);
    m_positionTimer->setInterval(10000);

    libvlc_log_set(m_vlcInstance, vlcLogCallback, nullptr);
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
        libvlc_media_player_set_hwnd(m_mediaPlayer, (void*)handle);
    }
#elif defined(Q_OS_LINUX)
    libvlc_media_player_set_xwindow(m_mediaPlayer, window->winId());
#elif defined(Q_OS_MAC)
    libvlc_media_player_set_nsobject(m_mediaPlayer, (void*)window->winId());
#endif

    QRectF rect = videoOutput->mapRectToScene(videoOutput->boundingRect());
    libvlc_video_set_scale(m_mediaPlayer, 0);
}






// In VLCPlayerHandler.h, add:
struct SubtitleTrack {
    int id;
    QString path;
    QString language;
    QString name;
};
// Add to private members:
QList<SubtitleTrack> m_loadedSubtitles;
int m_nextSubtitleId;

// In VLCPlayerHandler.cpp:
void VLCPlayerHandler::loadMedia(const QString& mediaId) {
    if (!verifyVLCSetup()) {
        return;
    }

    m_currentMediaId = mediaId;
    m_subtitleTracks.clear();
    m_loadedSubtitles.clear();
    m_nextSubtitleId = 2;  // Start from 1 since -1 is reserved for "No subtitles"
    emit subtitleTracksChanged();

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
        //libvlc_media_add_option(m_media, ":demux=adaptive");
        libvlc_media_add_option(m_media, ":adaptive-formats=dash");

        libvlc_media_player_set_media(m_mediaPlayer, m_media);

        playMedia();

        // Load subtitles synchronously
        tryLoadSubtitle(mediaId, "en");
        tryLoadSubtitle(mediaId, "es");

        

        
        startSubtitleMonitoring();
        
        emit mediaLoaded();
        
        

        
    }
    else {
        qDebug() << "Failed to create media";
        emit errorOccurred("Failed to create media");
    }
}

void VLCPlayerHandler::tryLoadSubtitle(const QString& mediaId, const QString& language) {
    QUrl url(QString("http://127.0.0.1:18080/media/%1/subtitles/%2").arg(mediaId, language + ".vtt"));

    if (m_mediaPlayer) {
        // Store subtitle information


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



        if (result == 0) {
            qDebug() << "Added subtitle track:" << language;
        }
        else {
            qWarning() << "Failed to add subtitle track:" << language << "Error code:" << result;
        }
    }
    else {
        qWarning() << "Media player is not initialized.";
    }
}



void VLCPlayerHandler::updateSubtitleTracks() {
    m_subtitleTracks.clear();

    // Add "No subtitles" option
    QVariantMap noneTrack;
    noneTrack["id"] = -1;
    noneTrack["name"] = "No subtitles";
    m_subtitleTracks.append(noneTrack);


    libvlc_track_description_t* tracks = libvlc_video_get_spu_description(m_mediaPlayer);
    libvlc_track_description_t* currentTrack = tracks;

    // Assuming the new subtitle is the last one added
    int newTrackId = -1;
    QString trackName;

    bool added_en = false;

    if (currentTrack) {
        while (currentTrack) {
            newTrackId = currentTrack->i_id;
            trackName = QString::fromUtf8(currentTrack->psz_name);

            if (newTrackId != -1) {
                // Store subtitle information
                SubtitleTrack track;
                track.id = newTrackId;
                if (added_en) {
                    track.language = "es";
                    track.name = "Espanol";
                } else {
                    track.language = "en";
                    track.name = "English";
                    added_en = true;
                }
                
                qDebug() << "Added subtitle track:" << track.name << "with ID:" << track.id;
                m_loadedSubtitles.append(track);
            }


            currentTrack = currentTrack->p_next;
        }

        

        
    }
    else {
        qWarning() << "Failed to retrieve subtitle tracks.";
    }

    // Release the track descriptions
    libvlc_track_description_list_release(tracks);

    // Add our loaded subtitle tracks
    for (const SubtitleTrack& track : m_loadedSubtitles) {
        QVariantMap trackInfo;
        trackInfo["id"] = track.id;
        trackInfo["name"] = track.language == "en" ? "English" : "Spanish";
        m_subtitleTracks.append(trackInfo);
    }

    emit subtitleTracksChanged();
}



void VLCPlayerHandler::setSubtitleTrack(int trackId) {
    qDebug() << "\n=== Starting setSubtitleTrack ===";
    qDebug() << "Requested track ID:" << trackId;

    if (!m_mediaPlayer) {
        qDebug() << "ERROR: Media player is null!";
        return;
    }
    qDebug() << libvlc_video_get_spu_count(m_mediaPlayer);
    if (trackId == -1) {
        qDebug() << "Disabling subtitles";
        libvlc_video_set_spu(m_mediaPlayer, -1);
        return;
    }

    for (int i = 0; i < 2; i++) {
        libvlc_track_description_t* tracks = libvlc_video_get_spu_description(m_mediaPlayer);

        if (!tracks) {
            qDebug() << "ERROR: No subtitle tracks available!";
        }
        else {
            qDebug() << "Available subtitle tracks:";

            // Iterate over the linked list of track descriptions
            libvlc_track_description_t* currentDesc = tracks;
            while (currentDesc) {
                qDebug() << "Track ID:" << currentDesc->i_id << "Name:" << currentDesc->psz_name;
                currentDesc = currentDesc->p_next;
            }

            // Release the memory used by the track descriptions
            libvlc_track_description_list_release(tracks);
        }
    
    }

    int currentTrack = libvlc_video_get_spu(m_mediaPlayer);
    qDebug() << "Current subtitle track after setting:" << currentTrack;
    int result = libvlc_video_set_spu(m_mediaPlayer, trackId);
    qDebug() << "Set subtitle result:" << result;
    currentTrack = libvlc_video_get_spu(m_mediaPlayer);
    qDebug() << "Current subtitle track after setting:" << currentTrack;

    qDebug() << "=== End setSubtitleTrack ===\n";
}

void VLCPlayerHandler::disableSubtitles() {
    if (m_mediaPlayer) {
        libvlc_video_set_spu(m_mediaPlayer, -1);
    }
}

QVariantList VLCPlayerHandler::subtitleTracks() const {
    return m_subtitleTracks;
}

void VLCPlayerHandler::onMediaStateChanged() {
    int mediaState = libvlc_media_player_get_state(m_mediaPlayer);

    // Check if media is fully loaded and parsed
    if (mediaState == libvlc_Ended || mediaState == libvlc_Playing || mediaState == libvlc_Paused) {
        int spuCount = libvlc_video_get_spu_count(m_mediaPlayer);
        if (spuCount > 0) {
            qDebug() << "Subtitles are now available.";
            updateSubtitleTracks();
            return;  // Stop further checks if successful
        }
    }

    // Recheck until loaded
    QTimer::singleShot(500, this, &VLCPlayerHandler::onMediaStateChanged);
}

void VLCPlayerHandler::startSubtitleMonitoring() {
    // Trigger state monitoring
    onMediaStateChanged();
}