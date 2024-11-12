#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQuickStyle>
#include <QtQuickControls2>
#include "Login.h"


int main(int argc, char* argv[])
{
#if defined(Q_OS_WIN) && QT_VERSION_CHECK(5, 6, 0) <= QT_VERSION && QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
#endif

    QGuiApplication app(argc, argv);

    // Set the style before creating any QML components
    QQuickStyle::setStyle("Basic");  // Use Basic style which allows full customization

    // Register Login class
    qmlRegisterType<Login>("com.ghoststream", 1, 0, "Login");

    QQmlApplicationEngine engine;
    engine.load(QUrl(QStringLiteral("qrc:/qt/qml/ghostclient/main.qml")));
    if (engine.rootObjects().isEmpty())
        return -1;

    return app.exec();
}