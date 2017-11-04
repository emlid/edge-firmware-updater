#include <QtRemoteObjects>
#include <QStandardPaths>
#include <QtCore>

#include "FirmwareUpgraderWatcher.h"
#include <iostream>


void messageHandler(QtMsgType type, const QMessageLogContext& context, QString const& msg)
{
    Q_UNUSED(context);

    static auto  logFilename = QStandardPaths::writableLocation(QStandardPaths::HomeLocation) + "/fw_log";
    static QFile logFile(logFilename);

    if (!logFile.isOpen()) {
        auto successful = logFile.open(QIODevice::WriteOnly);
        if (!successful) {
            QTextStream(stderr) << "Can not open log file " << logFilename;
            return;
        }
    }

    QTextStream logDataStream(&logFile);
    QTextStream errDataStream(stderr);

    auto sendlog =
        [&logDataStream, &errDataStream, &msg]
            (QString const& prefix) -> void {
                auto mess = prefix + msg;
                logDataStream << mess;
                errDataStream << mess;
            };

    switch (type) {
        case QtDebugMsg:
            sendlog("debug: ");
            break;

        case QtCriticalMsg:
            sendlog("critical: ");
            break;

        case QtFatalMsg:
            sendlog("fatal: ");
            std::exit(1);
            break;

        case QtWarningMsg:
            sendlog("warn: ");
            break;

        case QtInfoMsg:
            sendlog("info: ");
            break;
        default:
            break;
    }

    logDataStream.flush();
}


int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    qInstallMessageHandler(messageHandler);

    auto serverUrl = QUrl(QStringLiteral("local:ed"));
    QRemoteObjectHost serverNode(serverUrl);

    // Remote our watcher to other processes
    FirmwareUpgraderWatcher watcher;
    auto successful = serverNode.enableRemoting(&watcher);

    if (successful) {
        qInfo() << "Remoting started.";
    } else {
        qCritical() << "Remoting failed.";
        std::exit(EXIT_FAILURE);
    }

    return a.exec();
}
