#include <QCoreApplication>
#include <QtCore>
#include <iostream>

#include "testnotifier.h"
#include "qioflasher.h"

#include "StorageDeviceManager.h"

#include <windows.h>
#include <io.h>
#include <fcntl.h>


int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    auto manager = StorageDeviceManager::instance();

    auto physicalDrives = manager->physicalDrives(2316, 4096);

    for (std::shared_ptr<StorageDevice> device : physicalDrives) {
        QTextStream(stdout) << device->toString();
        if (device->unmountAllMountpoints().failed()) {
           QTextStream(stdout) << "Unmount failed" << '\n';
           break;
        }

        int fd;
        if (device->open(&fd).failed()) {
           QTextStream(stdout) << "Open device failed";
           break;
        }
        QFile src("C:\\Users\\vladimir.provalov\\Downloads\\emrasp.img");
        src.open(QIODevice::ReadOnly);

        QFile fl;
        bool res = fl.open(fd, QIODevice::WriteOnly);
        if (!res) {
            qDebug() << "Can not open phys drive for write";
            break;
        }

        QIOFlasher flasher;
        TestNotifier notifier(flasher);

        flasher.flash(src, fl, device-> recommendedBlockSize());
    }

    return a.exec();
}
