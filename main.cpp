#include <QtRemoteObjects>
#include <QtCore>

#include "FirmwareUpgraderWatcher.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    qSetMessagePattern("%{type} : %{message}");

    auto serverUrl = QUrl(QStringLiteral("local:fiw_upgrader"));
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
