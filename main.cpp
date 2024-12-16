#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQuickStyle>
#include <QtQuickControls2>
#include <QQmlContext>
#include "Login.h"
#include "VLCPlayerHandler.h"  
#include "MediaPlayer.h"
#include <Navigator.h>





int main(int argc, char* argv[])
{
#if defined(Q_OS_WIN) && QT_VERSION_CHECK(5, 6, 0) <= QT_VERSION && QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
#endif
    QGuiApplication::setHighDpiScaleFactorRoundingPolicy(Qt::HighDpiScaleFactorRoundingPolicy::Round); // Ensure consistent scaling

    QGuiApplication app(argc, argv);
    QQuickStyle::setStyle("Fusion");

    QGuiApplication::setAttribute(Qt::AA_Use96Dpi, true); // Treat FullHD (1920x1080) as baseline resolution
    qputenv("QT_SCALE_FACTOR", "1.0"); // Set scale factor to 1.0


    // Register types with the correct template parameters
    qmlRegisterType<Login>("com.ghoststream", 1, 0, "Login");
    qmlRegisterType<VLCPlayerHandler>("com.ghoststream", 1, 0, "VLCPlayerHandler");
    qmlRegisterType<MediaPlayer>("com.ghoststream", 1, 0, "MediaPlayer");
    //qmlRegisterType<MediaFilterHandler>("com.ghoststream", 1, 0, "MediaFilterHandler");
    qmlRegisterType<Navigator>("com.ghoststream", 1, 0, "Navigator");

    QQmlApplicationEngine engine;
    engine.load(QUrl(QStringLiteral("qrc:/qt/qml/ghostclient/main.qml")));
    if (engine.rootObjects().isEmpty())
        return -1;

    return app.exec();
}