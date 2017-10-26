#include <QtRemoteObjects>
#include <QtCore>

#include "RemoteFlasherWatcher.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    qSetMessagePattern("%{type} : %{message}");

    auto serverUrl = QUrl(QStringLiteral("local:edge_fw_upgrader"));
    QRemoteObjectHost serverNode(serverUrl);

    // Remote our watcher to other processes
    RemoteFlasherWatcher watcher;
    serverNode.enableRemoting(&watcher);

    return a.exec();
}
