#include <QtRemoteObjects>
#include <QStandardPaths>
#include <QtCore>

#include <iostream>

#include "utilities.h"
#include "EdgeFirmwareUpdater.h"


static QFile* logFile = nullptr;


static void logHandler(QtMsgType msgType, QMessageLogContext const& ctx, QString const& msg)
{
    updater::messageHandler(logFile, msgType, ctx, msg);
}


QString updaterNodeName(void) {
    return updater::shared::properties.updaterReplicaNodeName;
}


int main(int argc, char *argv[])
{
    // Register Metatypes
    qRegisterMetaType<updater::shared::LogMessageType>("updater::shared::LogMessageType");
    qRegisterMetaType<updater::shared::OperationStatus>("updater::shared::OperationStatus");

    QCoreApplication a(argc, argv);
    logFile = new QFile("/tmp/.fwupgrader.log");

    if (!logFile->open(QIODevice::WriteOnly)) {
        qCWarning(logg::basic()) << "Can not open file for logging";
    } else {
        // Register log message handler
        ::qInstallMessageHandler(logHandler);
    }


    updater::perm::reducePriviledge();

    QRemoteObjectHost serverNode(updaterNodeName());

    // Remote our watcher to other processes
    EdgeFirmwareUpdater watcher;
    auto successful = serverNode.enableRemoting(&watcher);

    if (successful) {
        qCInfo(logg::basic()) << "Remoting started.";
    } else {
        qCCritical(logg::basic()) << "Remoting failed.";
        std::exit(EXIT_FAILURE);
    }

    updater::perm::reduceRObjectNodePermissions(
        ::updaterNodeName()
    );

    return a.exec();
}
