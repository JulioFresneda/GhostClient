#ifndef VLCPLAYERHANDLER_H
#define VLCPLAYERHANDLER_H

#include <memory>
#include <vlc/vlc.h>
#include <QObject>
#include <QTimer>

class VLCPlayerHandler : public QObject
{
    Q_OBJECT

public:
    explicit VLCPlayerHandler(QObject* parent = nullptr);
    ~VLCPlayerHandler();

    Q_INVOKABLE void setMedia(const QString& mediaUrl);
    Q_INVOKABLE void play();
    Q_INVOKABLE void pause();
    Q_INVOKABLE void stop();
    Q_INVOKABLE void setSubtitleTrack(int trackId);
    Q_INVOKABLE void disableSubtitles();

signals:
    void mediaStateChanged();

private:
    void releaseMedia();

    std::unique_ptr<libvlc_instance_t, void(*)(libvlc_instance_t*)> vlcInstance;
    std::unique_ptr<libvlc_media_player_t, void(*)(libvlc_media_player_t*)> mediaPlayer;
    libvlc_media_t* media;
    QTimer* positionTimer;
};

#endif // VLCPLAYERHANDLER_H
