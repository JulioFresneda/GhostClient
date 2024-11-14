#ifndef VLCPLAYERHANDLER_H
#define VLCPLAYERHANDLER_H

#include <QObject>
#include <QString>
#include <QVideoSink>
#include <QTimer>
#include <QJsonObject>
#include <QJsonDocument>
#include <vlc/vlc.h>
#include <QQuickItem>

class VLCPlayerHandler : public QObject {
    Q_OBJECT
        Q_PROPERTY(qint64 duration READ duration NOTIFY durationChanged)
        Q_PROPERTY(qint64 position READ position WRITE setPosition NOTIFY positionChanged)
        Q_PROPERTY(bool isPlaying READ isPlaying NOTIFY playingStateChanged)
        Q_PROPERTY(QVideoSink* videoSink READ videoSink WRITE setVideoSink NOTIFY videoSinkChanged)
        

public:
    explicit VLCPlayerHandler(QObject* parent = nullptr);
    ~VLCPlayerHandler();

    // Essential getters
    qint64 duration() const;
    qint64 position() const;
    bool isPlaying() const;
    QVideoSink* videoSink() const;

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

private:
    
    void initializeVLC();
    void cleanupVLC();
    void updateMediaInfo();
    QString constructManifestUrl(const QString& mediaId);
    bool verifyVLCSetup();

    libvlc_instance_t* m_vlcInstance;
    libvlc_media_player_t* m_mediaPlayer;
    libvlc_media_t* m_media;

    bool m_isPlaying;
    QString m_token;
    QString m_userId;
    QVideoSink* m_videoSink;
    QTimer* m_positionTimer;
};

#endif // VLCPLAYERHANDLER_H