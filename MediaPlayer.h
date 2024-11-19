#ifndef MEDIAPLAYER_H
#define MEDIAPLAYER_H

#include <QQuickItem>
#include <QString>
#include <QVideoSink>
#include <QQmlEngine>
#include "VLCPlayerHandler.h"

class MediaPlayer : public QQuickItem
{
    Q_OBJECT
        QML_ELEMENT

        // Basic properties
        Q_PROPERTY(QString mediaId READ mediaId WRITE setMediaId NOTIFY mediaIdChanged)
        Q_PROPERTY(QString title READ title WRITE setTitle NOTIFY titleChanged)
        Q_PROPERTY(QVideoSink* videoSink READ videoSink WRITE setVideoSink NOTIFY videoSinkChanged)

        // Playback state properties
        Q_PROPERTY(bool isPlaying READ isPlaying NOTIFY isPlayingChanged)
        Q_PROPERTY(qint64 position READ position NOTIFY positionChanged)
        Q_PROPERTY(qint64 duration READ duration NOTIFY durationChanged)
        Q_PROPERTY(int volume READ volume WRITE setVolume NOTIFY volumeChanged)

public:
    explicit MediaPlayer(QQuickItem* parent = nullptr);

    // Property getters
    QString mediaId() const { return m_mediaId; }
    QString title() const { return m_title; }
    QVideoSink* videoSink() const { return m_videoSink; }
    bool isPlaying() const;
    qint64 position() const;
    qint64 duration() const;
    int volume() const;

    // Property setters
    void setMediaId(const QString& mediaId);
    void setTitle(const QString& title);
    void setVideoSink(QVideoSink* sink);
    void setVolume(int volume);

    // Player control methods
    Q_INVOKABLE void play();
    Q_INVOKABLE void pause();
    Q_INVOKABLE void stop();
    Q_INVOKABLE void seek(qint64 position);
    Q_INVOKABLE void setSubtitles(int trackId);
    Q_INVOKABLE void disableSubtitles();
    Q_INVOKABLE void closePlayer();

signals:
    // Property change signals
    void mediaIdChanged();
    void titleChanged();
    void videoSinkChanged();
    void isPlayingChanged();
    void positionChanged();
    void durationChanged();
    void volumeChanged();

    // State and control signals
    void mediaStateChanged();
    void closeRequested();
    void error(const QString& message);

private slots:
    void handleMediaStateChanged();

private:
    void initializePlayer();

    // Media properties
    QString m_mediaId;
    QString m_title;
    QString m_manifestUrl;

    // Authentication properties
    QString m_storedToken;
    QString m_userID;
    QString m_selectedProfileID;

    // Player state
    bool m_isInitialized;
    bool m_isPlaying;

    // Video output
    QVideoSink* m_videoSink;

    // VLC player instance
    VLCPlayerHandler m_player;
};

#endif // MEDIAPLAYER_H