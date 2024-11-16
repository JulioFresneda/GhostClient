#include "FFplayHandler.h"
#include <QSettings>
#include <QDebug>

FFplayHandler::FFplayHandler(QObject* parent)
    : QObject(parent)
    , m_ffplayProcess(new QProcess(this))
    , m_videoWidget(nullptr)
    , m_isPlaying(false)
    , m_position(0)
{
    QSettings settings("./conf.ini", QSettings::IniFormat);
    m_token = settings.value("authToken").toString();
    m_userId = settings.value("userID").toString();

    m_positionTimer = new QTimer(this);
    connect(m_positionTimer, &QTimer::timeout, this, &FFplayHandler::updatePosition);
    m_positionTimer->setInterval(1000);

    connect(m_ffplayProcess, &QProcess::started, this, &FFplayHandler::onFFplayStarted);
    connect(m_ffplayProcess, &QProcess::errorOccurred, this, &FFplayHandler::onFFplayError);
    connect(m_ffplayProcess, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
        this, &FFplayHandler::onFFplayFinished);
}

FFplayHandler::~FFplayHandler() {
    cleanup();
}

void FFplayHandler::loadMedia(const QString& mediaId) {
    cleanup();

    if (!m_videoWidget) {
        emit errorOccurred("No video widget set");
        return;
    }

    QString url = constructUrl(mediaId);
    QStringList arguments;

    // Set the window ID for FFplay to render into
#ifdef Q_OS_WIN
    arguments << "-window_title" << "GhostStream"
        << "-x" << QString::number(m_videoWidget->width())
        << "-y" << QString::number(m_videoWidget->height())
        << "-left" << QString::number(m_videoWidget->x())
        << "-top" << QString::number(m_videoWidget->y())
        << "-window" << QString::number((qint64)m_videoWidget->winId())
        << "-autoexit"
        << url;
#else
    arguments << "-window_title" << "GhostStream"
        << "-x" << QString::number(m_videoWidget->width())
        << "-y" << QString::number(m_videoWidget->height())
        << "-wid" << QString::number((qint64)m_videoWidget->winId())
        << "-autoexit"
        << url;
#endif

    m_ffplayProcess->start("ffplay", arguments);
}

void FFplayHandler::playMedia() {
    if (m_ffplayProcess->state() == QProcess::Running) {
        m_ffplayProcess->write(" ");  // Space key for play/pause
        m_isPlaying = true;
        m_positionTimer->start();
        emit playingStateChanged(true);
    }
}

void FFplayHandler::pauseMedia() {
    if (m_ffplayProcess->state() == QProcess::Running) {
        m_ffplayProcess->write(" ");  // Space key for play/pause
        m_isPlaying = false;
        m_positionTimer->stop();
        emit playingStateChanged(false);
    }
}

void FFplayHandler::stop() {
    cleanup();
}

void FFplayHandler::setVideoWidget(QWidget* widget) {
    if (m_videoWidget != widget) {
        m_videoWidget = widget;
        emit videoWidgetChanged();
    }
}

void FFplayHandler::setPosition(qint64 position) {
    if (m_position != position && m_ffplayProcess->state() == QProcess::Running) {
        m_position = position;
        // Convert position to seconds and send seek command to ffplay
        int seconds = position / 1000;
        m_ffplayProcess->write(QString::number(seconds).toLocal8Bit());
        m_ffplayProcess->write("\n");
        emit positionChanged(position);
    }
}

void FFplayHandler::cleanup() {
    if (m_ffplayProcess->state() != QProcess::NotRunning) {
        m_ffplayProcess->kill();
        m_ffplayProcess->waitForFinished();
    }
    m_positionTimer->stop();
    m_isPlaying = false;
    m_position = 0;
    emit playingStateChanged(false);
    emit positionChanged(0);
}

QString FFplayHandler::constructUrl(const QString& mediaId) {
    return QString("http://localhost:18080/media/%1/manifest?userID=%2&token=%3")
        .arg(mediaId)
        .arg(m_userId)
        .arg(m_token);
}

void FFplayHandler::onFFplayStarted() {
    m_isPlaying = true;
    m_positionTimer->start();
    emit playingStateChanged(true);
    emit mediaLoaded();
}

void FFplayHandler::onFFplayError(QProcess::ProcessError error) {
    qDebug() << "FFplay error:" << error;
    emit errorOccurred("FFplay process error: " + QString::number(error));
    cleanup();
}

void FFplayHandler::onFFplayFinished(int exitCode, QProcess::ExitStatus exitStatus) {
    qDebug() << "FFplay finished with exit code:" << exitCode;
    cleanup();
}

void FFplayHandler::updatePosition() {
    m_position += 1000;
    emit positionChanged(m_position);
}