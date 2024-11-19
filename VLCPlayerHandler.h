#ifndef VLCPLAYERHANDLER_H
#define VLCPLAYERHANDLER_H

#include <vlc/vlc.h>
#include <QObject>
#include <QString>
#include <QVideoSink>
#include <QTimer>
#include <QQuickItem>
#include <QNetworkAccessManager>
#include <QNetworkReply>

class VLCPlayerHandler : public QObject {
    Q_OBJECT
        Q_PROPERTY(qint64 duration READ duration NOTIFY durationChanged)
        Q_PROPERTY(qint64 position READ position WRITE setPosition NOTIFY positionChanged)
        Q_PROPERTY(bool isPlaying READ isPlaying NOTIFY playingStateChanged)
        Q_PROPERTY(QVideoSink* videoSink READ videoSink WRITE setVideoSink NOTIFY videoSinkChanged)
        Q_PROPERTY(QVariantList subtitleTracks READ subtitleTracks NOTIFY subtitleTracksChanged)

public:
    explicit VLCPlayerHandler(QObject* parent = nullptr);
    ~VLCPlayerHandler();

    qint64 duration() const;
    qint64 position() const;
    bool isPlaying() const;
    QVideoSink* videoSink() const;
    QVariantList subtitleTracks() const;

    Q_INVOKABLE void setSubtitleTrack(int trackId);
    Q_INVOKABLE void disableSubtitles();

public slots:
    void attachVideoOutput(QQuickItem* videoOutput);
    void setPosition(qint64 position);
    void setVideoSink(QVideoSink* sink);
    void playMedia();
    void pauseMedia();
    void loadMedia(const QString& mediaId);
    void stop();

signals:
    void durationChanged(qint64 duration);
    void positionChanged(qint64 position);
    void playingStateChanged(bool isPlaying);
    void errorOccurred(const QString& error);
    void mediaLoaded();
    void videoSinkChanged();
    void subtitleTracksChanged();

private:
    void cleanupVLC();
    void updateMediaInfo();
    bool verifyVLCSetup();
    void tryLoadSubtitle(const QString& mediaId, const QString& language);
    void updateSubtitleTracks();

    libvlc_instance_t* m_vlcInstance;
    libvlc_media_player_t* m_mediaPlayer;
    libvlc_media_t* m_media;
    QString m_currentMediaId;
    bool m_isPlaying;
    QString m_token;
    QString m_userId;
    QVideoSink* m_videoSink;
    QTimer* m_positionTimer;
    QVariantList m_subtitleTracks;
    QNetworkAccessManager m_networkManager;
    int m_pendingSubtitles;
};

#endif // VLCPLAYERHANDLER_H