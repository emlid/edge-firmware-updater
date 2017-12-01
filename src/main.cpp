#include <QtRemoteObjects>
#include <QStandardPaths>
#include <QtCore>

#include <iostream>

#ifdef Q_OS_LINUX
#include <sys/types.h>
#include <sys/stat.h>
#endif

#include "FirmwareUpgraderWatcher.h"


void messageHandler(QtMsgType msgType, const QMessageLogContext& context, QString const& msg)
{
    Q_UNUSED(context);

    static auto  logFilename = QStandardPaths::writableLocation(QStandardPaths::HomeLocation)
            + "/.fwupgrader.log";
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
                auto mess = QTime::currentTime().toString() + ": " + prefix + msg + '\n';
                logDataStream << mess;
                errDataStream << mess;
            };

    switch (msgType) {
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


void reducePriviledge(void) {
#ifdef Q_OS_LINUX
    ::umask(0);
#endif
}


int main(int argc, char *argv[])
{
    qRegisterMetaType<states::DeviceScannerState>("states::DeviceScannerState");
    qRegisterMetaType<states::RpiBootState>("states::RpiBootState");
    qRegisterMetaType<states::FlasherState>("states::FlasherState");
    qRegisterMetaType<states::StateType>("states::StateType");
    qRegisterMetaType<states::CheckingCorrectnessState>("states::CheckingCorrectnessState");

    QCoreApplication a(argc, argv);
    qInstallMessageHandler(::messageHandler);

    ::reducePriviledge();

    auto serverUrl = QUrl(QStringLiteral("local:fwupg_socket"));
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
