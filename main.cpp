#include <QCoreApplication>
#include <QtCore>
#include <iostream>

#include "testnotifier.h"
#include "qioflasher.h"

#include "StorageDeviceManager.h"


int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    auto manager = StorageDeviceManager::instance();

    auto physicalDrives = manager->physicalDrives(2316, 4096);

    for (std::shared_ptr<StorageDevice> device : physicalDrives) {
        QTextStream(stdout) << device->toString();

        int fd = 0;
        if (device->open(&fd).failed()) {
            std::exit(0);
        }

        QFile dest;
        dest.open(fd, QIODevice::WriteOnly);

        QFile src("/home/vladimir.provalov/Downloads/emlid-raspbian-20170323.img");
        src.open(QIODevice::ReadOnly);

        QIOFlasher flasher;
        TestNotifier notif(flasher);

        flasher.flash(src, dest);
    }

    return a.exec();
}
