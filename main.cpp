#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQuickStyle>
#include <QtQuickControls2>
#include <QQmlContext>
#include "Medium.h"
#include "VLCPlayerHandler.h"  
#include <Navigator.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>

/**
 * Resolves the given domain name to an IP address.
 * If `localHost` is true, it will return "localhost" without performing a DNS lookup.
 *
 * @param domain The domain name to resolve.
 * @param localHost If true, resolves to "localhost". Default is true.
 * @return A QString containing the resolved IP address or an empty string on failure.
 */
QString resolveDomain(const QString& domain, bool localHost = false) {
    if (localHost) {
        return "localhost";
    }
    QDnsLookup dns;
    dns.setType(QDnsLookup::A); // Query for IPv4 address
    dns.setName(domain);

    // Event loop to wait for DNS query completion
    QEventLoop loop;
    QObject::connect(&dns, &QDnsLookup::finished, &loop, &QEventLoop::quit);

    dns.lookup();
    loop.exec(); // Block and wait for the query to finish

    // Handle lookup errors
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

/**
 * Main entry point for the application.
 * Configures the Qt application environment, initializes QML types, and loads the main QML file.
 *
 * @param argc The number of command-line arguments.
 * @param argv An array of command-line arguments.
 * @return The exit status of the application.
 */
int main(int argc, char* argv[]) {
#if defined(Q_OS_WIN) && QT_VERSION_CHECK(5, 6, 0) <= QT_VERSION && QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    // Enable high DPI scaling for Qt versions between 5.6 and 6.0 on Windows
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
#endif

    // Ensure consistent scaling policy
    QGuiApplication::setHighDpiScaleFactorRoundingPolicy(Qt::HighDpiScaleFactorRoundingPolicy::Round);

    QGuiApplication app(argc, argv);

    // Set application style to "Fusion"
    QQuickStyle::setStyle("Fusion");

    // Treat FullHD (1920x1080) as baseline resolution
    QGuiApplication::setAttribute(Qt::AA_Use96Dpi, true);
    qputenv("QT_SCALE_FACTOR", "1.0"); // Set scale factor to 1.0

    // Load configuration and resolve public IP address
    QSettings settings("./conf.ini", QSettings::IniFormat);
	QString domain = settings.value("domain", "localhost").toString();

    settings.setValue("publicIP", resolveDomain(domain, true));
    settings.sync();

    // Register custom QML types
    qmlRegisterType<Medium>("com.ghoststream", 1, 0, "Medium");
    qmlRegisterType<VLCPlayerHandler>("com.ghoststream", 1, 0, "VLCPlayerHandler");
    qmlRegisterType<Navigator>("com.ghoststream", 1, 0, "Navigator");

    // Initialize the QML application engine
    QQmlApplicationEngine engine;
    engine.load(QUrl(QStringLiteral("qrc:/qt/qml/ghostclient/main.qml")));

    // Exit if the QML engine failed to load the main file
    if (engine.rootObjects().isEmpty())
        return -1;

    return app.exec();
}
