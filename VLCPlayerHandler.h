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
    void startSubtitleMonitoring();

    Q_PROPERTY(QVariantList audioTracks READ audioTracks NOTIFY audioTracksChanged)
    QVariantList audioTracks() const;
    Q_INVOKABLE void setAudioTrack(int trackId);

    Q_INVOKABLE void setFullScreen(bool fullScreen);

    bool eventFilter(QObject* obj, QEvent* event) override;

    Q_INVOKABLE void setVolume(int volume);

public slots:
    void attachVideoOutput(QQuickItem* videoOutput);
    void setPosition(qint64 position);
    void setVideoSink(QVideoSink* sink);
    void playMedia(float percentage_watched);
    void pauseMedia();
    void loadMedia(const QString& mediaId, const QVariantMap& mediaMetadata);
    void stop();
    void forward30sec();
    void back30sec();

private slots:
    void onMediaStateChanged();

signals:
    void durationChanged(qint64 duration);
    void positionChanged(qint64 position);
    void playingStateChanged(bool isPlaying);
    void reallyPlayingStateChanged(bool isReallyPlaying);
    void errorOccurred(const QString& error);
    void mediaLoaded();
    void mediaEnded();
    void videoSinkChanged();
    void subtitleTracksChanged();
    void audioTracksChanged();

    void progressUpdated(float percentage);

private:
    void cleanupVLC();
    void updateMediaInfo();
    void updateMediaMetadataOnServer();
    bool verifyVLCSetup();
    bool tryLoadSubtitle(const QString& mediaId, const QString& language);
    void updateSubtitleTracks();
    void updateAudioTracks();

    void fetchMediaMetadata();

    libvlc_instance_t* m_vlcInstance;
    libvlc_media_player_t* m_mediaPlayer;
    libvlc_media_t* m_media;
    QString m_currentMediaId;
    QString m_currentSubtitlesCode;

    bool m_isPlaying;
    QString m_token;
    QString m_userId;
    QString m_profileId;
    QVideoSink* m_videoSink;
    QTimer* m_positionTimer;
    QTimer* m_metadataTimer;
    QTimer* m_loadingTimer;
    QVariantList m_subtitleTracks;
    QVariantList m_audioTracks;
    QNetworkAccessManager m_networkManager;
    int m_pendingSubtitles;

    double last_percentage_watched;

    QWindow* m_vlcWindow;
    QWindow* mainWindow;

    bool fullScreen;


};

#endif // VLCPLAYERHANDLER_H