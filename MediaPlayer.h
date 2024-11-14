// MediaPlayer.h
#ifndef MEDIAPLAYER_H
#define MEDIAPLAYER_H

#include <QObject>
#include <QQmlEngine>
#include "VLCPlayerHandler.h"

class MediaPlayer : public QObject {
    Q_OBJECT
        Q_PROPERTY(QString mediaId READ mediaId WRITE setMediaId NOTIFY mediaIdChanged)
        Q_PROPERTY(QString title READ title WRITE setTitle NOTIFY titleChanged)
        Q_PROPERTY(bool isPlaying READ isPlaying NOTIFY isPlayingChanged)
        Q_PROPERTY(qint64 position READ position WRITE setPosition NOTIFY positionChanged)
        Q_PROPERTY(qint64 duration READ duration NOTIFY durationChanged)

public:
    explicit MediaPlayer(QObject* parent = nullptr);

    QString mediaId() const { return m_mediaId; }
    QString title() const { return m_title; }
    bool isPlaying() const;
    qint64 position() const;
    qint64 duration() const;

    void setMediaId(const QString& mediaId);
    void setTitle(const QString& title);
    void setPosition(qint64 position);

public slots:
    void play();
    void pause();
    void stop();
    void close();

signals:
    void mediaIdChanged();
    void titleChanged();
    void isPlayingChanged();
    void positionChanged();
    void durationChanged();
    void closeRequested();

private:
    QString m_mediaId;
    QString m_title;
    VLCPlayerHandler* m_vlcPlayer;
};

#endif // MEDIAPLAYER_H
