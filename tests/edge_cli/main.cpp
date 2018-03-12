#include <QCoreApplication>
#include <edge.h>
#include <QtCore>

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    auto config = edge::EdgeConfig(
        0xa5c, 0x2764, 0x0001, "issue.txt", "temp_mnt", "boot"
    );

    auto edgeManager = edge::makeEdgeManager(config);

    while(!(edgeManager->isEdgePlugged() || edgeManager->isEdgeInitialized())) {
        QThread::msleep(500);
        qInfo() << "waiting for edge.";
    }

    qInfo() << "Edge plugged";
    auto edgeDevice = edgeManager->initialize();
    if (edgeDevice) {
        auto version = edgeDevice->firmwareVersion();
        qInfo() << "Firmware Version" << version;
    }

    return a.exec();
}
