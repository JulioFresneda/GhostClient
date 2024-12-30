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

/**
 * @brief Handles video playback using VLC backend in a Qt/QML application
 *
 * This class manages media playback, tracks (audio/subtitles), position control,
 * and video output rendering using libVLC. It provides a bridge between the QML UI
 * and the VLC media player functionality.
 */
class VLCPlayerHandler : public QObject {
    Q_OBJECT
        // Duration of the current media in milliseconds
        Q_PROPERTY(qint64 duration READ duration NOTIFY durationChanged)
        // Current playback position in milliseconds
        Q_PROPERTY(qint64 position READ position WRITE setPosition NOTIFY positionChanged)
        // Current playing state of the media
        Q_PROPERTY(bool isPlaying READ isPlaying NOTIFY playingStateChanged)
        // Video sink for rendering output
        Q_PROPERTY(QVideoSink* videoSink READ videoSink WRITE setVideoSink NOTIFY videoSinkChanged)
        // Available subtitle tracks
        Q_PROPERTY(QVariantList subtitleTracks READ subtitleTracks NOTIFY subtitleTracksChanged)
        // Available audio tracks
        Q_PROPERTY(QVariantList audioTracks READ audioTracks NOTIFY audioTracksChanged)

public:
    /**
     * @brief Constructs the VLC player handler
     * @param parent Parent QObject for memory management
     */
    explicit VLCPlayerHandler(QObject* parent = nullptr);

    /**
     * @brief Destructor that ensures proper cleanup of VLC resources
     */
    ~VLCPlayerHandler();

    /** @brief Gets the total duration of the current media */
    qint64 duration() const;

    /** @brief Gets the current playback position */
    qint64 position() const;

    /** @brief Returns whether media is currently playing */
    bool isPlaying() const;

    /** @brief Gets the current video sink */
    QVideoSink* videoSink() const;

    /** @brief Gets the list of available subtitle tracks */
    QVariantList subtitleTracks() const;

    /**
     * @brief Sets the active subtitle track
     * @param trackId ID of the subtitle track to activate
     */
    Q_INVOKABLE void setSubtitleTrack(int trackId);

    /** @brief Disables subtitle display */
    Q_INVOKABLE void disableSubtitles();

    /**
     * @brief Sets the active audio track
     * @param trackId ID of the audio track to activate
     */
    Q_INVOKABLE void setAudioTrack(int trackId);

    /** @brief Gets the index of the current audio track */
    Q_INVOKABLE int getAudioIndex();

    /** @brief Gets the description of the current audio track */
    Q_INVOKABLE QString getAudioText();

    /**
     * @brief Reorders a track list by track ID
     * @param list List to reorder
     * @param selectedTrackId ID of the track to prioritize
     */
    void reorderListById(QVariantList& list, int selectedTrackId);

    /** @brief Gets the list of available audio tracks */
    QVariantList audioTracks() const;

    /**
     * @brief Toggles fullscreen mode
     * @param fullScreen True to enable fullscreen, false to disable
     */
    Q_INVOKABLE void setFullScreen(bool fullScreen);

    /**
     * @brief Handles Qt events for the video window
     * @param obj Object receiving the event
     * @param event Event to process
     */
    bool eventFilter(QObject* obj, QEvent* event) override;

    /**
     * @brief Sets the playback volume
     * @param volume Volume level to set
     */
    Q_INVOKABLE void setVolume(int volume);

    /** @brief Updates metadata for the current media */
    Q_INVOKABLE void updateMediaMetadata();

    /**
     * @brief Loads a new media file
     * @param mediaId Unique identifier for the media
     * @param mediaMetadata Additional metadata for the media
     */
    Q_INVOKABLE void loadMedia(const QString& mediaId, const QVariantMap& mediaMetadata);

public slots:
    /**
     * @brief Attaches a video output surface
     * @param videoOutput QML video output item
     */
    void attachVideoOutput(QQuickItem* videoOutput);

    /**
     * @brief Sets the playback position
     * @param position Position in milliseconds
     */
    void setPosition(qint64 position);

    /**
     * @brief Sets the video sink for rendering
     * @param sink Video sink to use
     */
    void setVideoSink(QVideoSink* sink);

    /**
     * @brief Starts media playback
     * @param percentage_watched Starting position as percentage
     */
    void playMedia(float percentage_watched);

    /** @brief Pauses media playback */
    void pauseMedia();

    /** @brief Stops media playback */
    void stop();

    /** @brief Skips forward 30 seconds */
    void forward30sec();

    /** @brief Skips backward 30 seconds */
    void back30sec();

signals:
    /** @brief Emitted when media duration changes */
    void durationChanged(qint64 duration);

    /** @brief Emitted when playback position changes */
    void positionChanged(qint64 position);

    /** @brief Emitted when playing state changes */
    void playingStateChanged(bool isPlaying);

    /** @brief Emitted when actual playing state changes */
    void reallyPlayingStateChanged(bool isReallyPlaying);

    /** @brief Emitted when an error occurs */
    void errorOccurred(const QString& error);

    /** @brief Emitted when media is loaded */
    void mediaLoaded();

    /** @brief Emitted when media playback ends */
    void mediaEnded();

    /** @brief Emitted when video sink changes */
    void videoSinkChanged();

    /** @brief Emitted when subtitle tracks change */
    void subtitleTracksChanged();

    /** @brief Emitted when audio tracks change */
    void audioTracksChanged();

    /** @brief Emitted when playback progress updates */
    void progressUpdated(float percentage);

private:
    /** @brief Cleans up VLC resources */
    void cleanupVLC();

    /** @brief Updates media information */
    void updateMediaInfo();

    /** @brief Updates media metadata on the server */
    void updateMediaMetadataOnServer();

    /** @brief Verifies VLC setup is complete */
    bool verifyVLCSetup();

    /** @brief Attempts to download subtitles for media */
    bool tryDownloadSubtitles(const QString& mediaId);

    /** @brief Loads subtitle tracks with given preference */
    void loadSubtitleTracks(QString subtitles_chosen);

    /** @brief Loads audio tracks with given language preference */
    void loadAudioTracks(QString language_chosen);

    /** @brief Updates selected audio track status */
    void updateAudioSelected();

    /** @brief Updates selected subtitle track status */
    void updateSubtitleSelected();

    // VLC instance and player pointers
    libvlc_instance_t* m_vlcInstance;
    libvlc_media_player_t* m_mediaPlayer;
    libvlc_media_t* m_media;

    // Media identification and track info
    QString m_currentMediaId;
    QString m_currentSubtitlesText;
    int m_currentAudioId;
    int m_currentSubtitlesId;
    QString m_currentAudioText;
    QString m_url;

    // Playback state
    bool m_isPlaying;
    QString m_token;
    QString m_profileId;
    QVideoSink* m_videoSink;

    // Timers for various updates
    QTimer* m_positionTimer;
    QTimer* m_metadataTimer;
    QTimer* m_loadingTimer;

    // Track lists
    QVariantList m_subtitleTracks;
    QVariantList m_audioTracks;

    // Network handling
    QNetworkAccessManager m_networkManager;
    int m_pendingSubtitles;

    // Playback progress tracking
    double last_percentage_watched;

    // Window management
    QWindow* m_vlcWindow;
    QWindow* mainWindow;
    bool fullScreen;
};

#endif // VLCPLAYERHANDLER_H