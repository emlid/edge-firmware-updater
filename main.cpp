#include <QCoreApplication>
#include <QtCore>
#include <iostream>

#include "testnotifier.h"
#include "Flasher.h"
#include "StorageDeviceManager.h"
#include "rpiboot/rpiboot.h"


int main(int argc, char *argv[])
{
    qSetMessagePattern("%{type} : %{message}");

    QCoreApplication a(argc, argv);

    auto manager = StorageDeviceManager::instance();

    auto physicalDrives = manager->physicalDrives(0x0a5c, 0x2764);

    qInfo() << "devices count: " << physicalDrives.size();

    for (std::shared_ptr<StorageDevice> device : physicalDrives) {
        QTextStream(stdout) << device->toString();

        /*device->unmountAllMountpoints();

        QFile src("/home/vladimir.provalov/Downloads/emlid-raspbian-20170323.img");
        QFile dest;

        if (device->openAsQFile(&dest).failed()) {
            qCritical() << "Failed to open as qfile.";
            std::exit(1);
        }

        if (!src.open(QIODevice::ReadOnly)) {
            qCritical() << "Failed to open image.";
            std::exit(1);
        }

        Flasher flasher;
        TestNotifier notif(flasher);

        if (!flasher.flash(src, dest)) {
            QTextStream("failed");
        }*/
    }

    return a.exec();
}
