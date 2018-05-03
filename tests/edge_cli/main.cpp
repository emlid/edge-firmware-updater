#include <QCoreApplication>
#include <edge.h>
#include <QtCore>


int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    namespace conf = edge::tag;
    auto config = edge::EdgeConfig(
                conf::Vid{0xa5c},
                conf::Pid{0x2764},
                conf::Pid{0x0001},
                conf::VersionFileName{"issue.txt"},
                conf::MntptPathForBootPart{"temp_mnt"},
                conf::PartWithVersionFile{"boot"}
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
