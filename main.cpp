#include <QCoreApplication>
#include <QtCore>
#include <iostream>

#include "testnotifier.h"
#include "Flasher.h"
#include "StorageDeviceManager.h"
#include "rpiboot/rpiboot.h"


int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    auto manager = StorageDeviceManager::instance();

    int res = ::boot();

    QTextStream(stdout) << "successful\n";

    std::exit(0);

    auto physicalDrives = manager->physicalDrives(2316, 4096);

    for (std::shared_ptr<StorageDevice> device : physicalDrives) {
        QTextStream(stdout) << device->toString();

        device->unmountAllMountpoints();

        QFile src("C:\\Users\\vladimir.provalov\\Downloads\\emrasp.img");
        QFile dest;

        if (device->openAsQFile(&dest).failed()) {
            std::exit(1);
        }

        if (!src.open(QIODevice::ReadOnly)) {
            std::exit(1);
        }

        Flasher flasher;
        TestNotifier notif(flasher);

        if (!flasher.flash(src, dest)) {
            QTextStream("failed");
        }
    }

    return a.exec();
}
