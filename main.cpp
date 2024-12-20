#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQuickStyle>
#include <QtQuickControls2>
#include <QQmlContext>
#include "Medium.h"
#include "VLCPlayerHandler.h"  
#include "MediaPlayer.h"
#include <Navigator.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>

QString resolveDomain(const QString& domain, bool localHost = true) {
    if (localHost) {
        return "localhost";
    }
    QDnsLookup dns;
    dns.setType(QDnsLookup::A); // Query for IPv4 address
    dns.setName(domain);

    // Use an event loop to wait for the DNS query to complete
    QEventLoop loop;
    QObject::connect(&dns, &QDnsLookup::finished, &loop, &QEventLoop::quit);

    dns.lookup();
    loop.exec(); // Block and wait for the query to finish

    // Check if the lookup succeeded
    if (dns.error() != QDnsLookup::NoError) {
        std::cerr << "DNS Lookup error: " << dns.errorString().toStdString() << std::endl;
        return ""; // Return empty string on failure
    }

    // Return the first resolved IP address
    if (!dns.hostAddressRecords().isEmpty()) {
        return dns.hostAddressRecords().first().value().toString();
    }
    else {
        std::cerr << "No IP address found for domain: " << domain.toStdString() << std::endl;
        return "";
    }
}


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

    QSettings settings("./conf.ini", QSettings::IniFormat);
    settings.setValue("publicIP", resolveDomain("ghoststream.duckdns.org"));
    settings.sync();


    // Register types with the correct template parameters
    qmlRegisterType<Medium>("com.ghoststream", 1, 0, "Medium");
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