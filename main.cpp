#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQuickStyle>
#include <QtQuickControls2>
#include <QQmlContext>
#include "Login.h"
#include "VLCPlayerHandler.h"  
#include "MediaPlayer.h"



// TODO
// Features
// - Multiple audios, subtitles - OK
// - Controls design - OK
// - Autoplay next episode
// 
// Visuals
// - Load image
// - Categories



int main(int argc, char* argv[])
{
#if defined(Q_OS_WIN) && QT_VERSION_CHECK(5, 6, 0) <= QT_VERSION && QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
#endif

    QGuiApplication app(argc, argv);
    QQuickStyle::setStyle("Fusion");




    // Register types with the correct template parameters
    qmlRegisterType<Login>("com.ghoststream", 1, 0, "Login");
    qmlRegisterType<VLCPlayerHandler>("com.ghoststream", 1, 0, "VLCPlayerHandler");
    qmlRegisterType<MediaPlayer>("com.ghoststream", 1, 0, "MediaPlayer");

    QQmlApplicationEngine engine;
    engine.load(QUrl(QStringLiteral("qrc:/qt/qml/ghostclient/main.qml")));
    if (engine.rootObjects().isEmpty())
        return -1;

    return app.exec();
}