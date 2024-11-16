#ifndef FFPLAYHANDLER_H
#define FFPLAYHANDLER_H

#include <QObject>
#include <QProcess>
#include <QWidget>
#include <QTimer>

class FFplayHandler : public QObject {
    Q_OBJECT
        Q_PROPERTY(qint64 position READ position NOTIFY positionChanged)
        Q_PROPERTY(bool isPlaying READ isPlaying NOTIFY playingStateChanged)
        Q_PROPERTY(QWidget* videoWidget READ videoWidget WRITE setVideoWidget NOTIFY videoWidgetChanged)

public:
    Q_INVOKABLE explicit FFplayHandler(QObject* parent = nullptr);
    ~FFplayHandler();

    bool isPlaying() const { return m_isPlaying; }
    qint64 position() const { return m_position; }
    QWidget* videoWidget() const { return m_videoWidget; }

public slots:
    void loadMedia(const QString& mediaId);
    void playMedia();
    void pauseMedia();
    void stop();
    void setVideoWidget(QWidget* widget);
    void setPosition(qint64 position);

signals:
    void positionChanged(qint64 position);
    void playingStateChanged(bool playing);
    void videoWidgetChanged();
    void mediaLoaded();
    void errorOccurred(const QString& error);

private slots:
    void onFFplayStarted();
    void onFFplayError(QProcess::ProcessError error);
    void onFFplayFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void updatePosition();

private:
    QString constructUrl(const QString& mediaId);
    void cleanup();

    QProcess* m_ffplayProcess;
    QWidget* m_videoWidget;
    QTimer* m_positionTimer;
    bool m_isPlaying;
    qint64 m_position;
    QString m_token;
    QString m_userId;
};

#endif // FFPLAYHANDLER_H