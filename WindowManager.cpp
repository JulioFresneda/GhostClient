#include <QObject>
#include <QWindow>
#include <QQuickWindow>
#include <QQuickItem>
#include <QQmlComponent>
#include <QQmlContext>
#include <QQmlEngine>
#include <QScreen>

class WindowManager : public QObject {
    Q_OBJECT

public:
    explicit WindowManager(QObject* parent = nullptr) : QObject(parent) {
        // Create the main video window
        m_videoWindow = createWindow("qrc:/VideoOutput.qml", false);
        m_videoWindow->setTitle("Ghost Stream - Video");

        // Create the controls window with transparency
        m_controlsWindow = createWindow("qrc:/PlayerControls.qml", true);
        m_controlsWindow->setTitle("Ghost Stream - Controls");

        // Set up window flags for the controls window
        m_controlsWindow->setFlags(Qt::Window | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);

        // Connect window movement signals
        connect(m_videoWindow, &QQuickWindow::xChanged, this, &WindowManager::syncWindowPositions);
        connect(m_videoWindow, &QQuickWindow::yChanged, this, &WindowManager::syncWindowPositions);
        connect(m_videoWindow, &QQuickWindow::widthChanged, this, &WindowManager::syncWindowPositions);
        connect(m_videoWindow, &QQuickWindow::heightChanged, this, &WindowManager::syncWindowPositions);

        // Connect close signals
        connect(m_videoWindow, &QQuickWindow::closing, this, &WindowManager::closeWindows);
        connect(m_controlsWindow, &QQuickWindow::closing, this, &WindowManager::closeWindows);
    }

    ~WindowManager() {
        delete m_videoWindow;
        delete m_controlsWindow;
    }

    void show() {
        m_videoWindow->show();
        m_controlsWindow->show();
        syncWindowPositions();
    }

    void setMediaInfo(const QString& mediaId, const QString& title) {
        // Set media info for both windows
        QQmlContext* videoContext = QQmlEngine::contextForObject(m_videoWindow->contentItem());
        QQmlContext* controlsContext = QQmlEngine::contextForObject(m_controlsWindow->contentItem());

        if (videoContext && controlsContext) {
            videoContext->setContextProperty("mediaId", mediaId);
            videoContext->setContextProperty("title", title);
            controlsContext->setContextProperty("mediaId", mediaId);
            controlsContext->setContextProperty("title", title);
        }
    }

private slots:
    void syncWindowPositions() {
        if (!m_videoWindow || !m_controlsWindow) return;

        // Get the video window's geometry
        QRect videoGeometry = m_videoWindow->geometry();

        // Update the controls window position and size
        m_controlsWindow->setGeometry(
            videoGeometry.x(),
            videoGeometry.y(),
            videoGeometry.width(),
            videoGeometry.height()
        );
    }

    void closeWindows() {
        m_videoWindow->close();
        m_controlsWindow->close();
    }

private:
    QQuickWindow* createWindow(const QString& qmlPath, bool transparent) {
        QQmlEngine* engine = new QQmlEngine(this);
        QQmlComponent component(engine, QUrl(qmlPath));
        
        if (component.isError()) {
            qDebug() << "Error loading component:" << component.errors();
            return nullptr;
        }

        QObject* object = component.create();
        QQuickWindow* window = qobject_cast<QQuickWindow*>(object);
        
        if (!window) {
            QQuickItem* item = qobject_cast<QQuickItem*>(object);
            if (item) {
                window = new QQuickWindow();
                item->setParentItem(window->contentItem());
            }
        }

        if (window && transparent) {
            window->setColor(Qt::transparent);
        }

        return window;
    }

    QQuickWindow* m_videoWindow;
    QQuickWindow* m_controlsWindow;
};