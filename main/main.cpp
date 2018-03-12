#include <QtRemoteObjects>
#include <QStandardPaths>
#include <QtCore>

#include <iostream>

#if defined(Q_OS_LINUX) || defined(Q_OS_MACX)
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#endif

#include "EdgeFirmwareUpdater.h"


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
                logDataStream.flush();
                errDataStream.flush();
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
            std::abort();
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
}


void reducePriviledge(void) {
#if defined(Q_OS_LINUX) || defined(Q_OS_MACX)
    ::umask(0);
#endif
}


void chownSocket(void) {
#ifdef Q_OS_MACX
    auto env = QProcessEnvironment::systemEnvironment();

    auto uid = -1;
    auto gid = -1;
    auto euid = ::geteuid();

    if (env.contains("SUDO_UID")) {
        uid = env.value("SUDO_UID").toInt();
    }

    if (env.contains("SUDO_GID")) {
        gid = env.value("SUDO_GID").toInt();
    }

    ::chown("/tmp/fwupg_socket", uid, gid);

    qInfo() << "OSX: uid: " << uid << " euid:" << euid << "gid" << gid;
#endif
}


QString updaterSocketName(void) {
#ifdef Q_OS_MACX
    return "local:/tmp/fwupg_socket";
#else
    return "local:fwupg_socket";
#endif
}


int main(int argc, char *argv[])
{
    qRegisterMetaType<updater::shared::LogMessageType>("updater::shared::LogMessageType");
    qRegisterMetaType<updater::shared::OperationStatus>("updater::shared::OperationStatus");

    QCoreApplication a(argc, argv);
    qInstallMessageHandler(::messageHandler);

    ::reducePriviledge();

    QRemoteObjectHost serverNode(updaterSocketName());

    // Remote our watcher to other processes
    EdgeFirmwareUpdater watcher;
    auto successful = serverNode.enableRemoting(&watcher);

    if (successful) {
        qInfo() << "Remoting started.";
    } else {
        qCritical() << "Remoting failed.";
        std::exit(EXIT_FAILURE);
    }

    chownSocket();

    return a.exec();
}
