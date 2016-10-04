#include "firmwareupgradecontroller.h"
#include "rpiboot.h"
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "libusb-1.0/libusb.h"
#include <handledevice_win.h>
#include <QDebug>

void DeviceSearcher::startFindBoardLoop()
{
    emit searcherMessage("Scan for devices...");

    int bootable = findBootableDevices();
    qDebug() << "bootable:" << bootable;

    if (bootable > 0) {

        QThread rpibootThread;
        emit searcherMessage(QString("Found %1 bootable device%2").arg(bootable).arg(bootable > 1 ? "s":""));

        emit searcherMessage("Rpiboot started");
        startRpiBoot(&rpibootThread);
        rpibootThread.wait();
        emit searcherMessage("Rpiboot finished");
    }

    enumerateDevices();

    emit searcherMessage("Search finished");
    emit searchFinished();
}

int DeviceSearcher::findBootableDevices()
{
    libusb_context *context = 0;
    libusb_device **list = 0;
    int ret = 0;
    ssize_t count = 0;
    int bootable = 0;

    ret = libusb_init(&context);
    Q_ASSERT(ret == 0);

    count = libusb_get_device_list(context, &list);
    Q_ASSERT(count > 0);

    for (ssize_t idx = 0; idx < count; ++idx) {
        libusb_device *device = list[idx];
        struct libusb_device_descriptor desc;

        ret = libusb_get_device_descriptor(device, &desc);
        Q_ASSERT(ret == 0);

        // FIXME: remove hardcode from conditions
        if (desc.idVendor == 2652 && (desc.idProduct == 10083 || desc.idProduct == 10084)) {
           bootable++;
        }
    }

    libusb_exit(context);
    return bootable;
}


void DeviceSearcher::startUdevMonitor(int count)
{
    /*
     *  this method should be removed from Windows part of application
     */
}

void DeviceSearcher::enumerateDevices()
{
        // GetLogicalDrives returns 0 on failure, or a bitmask representing
        // the drives available on the system (bit 0 = A:, bit 1 = B:, etc)
        unsigned long driveMask = GetLogicalDrives();
        int i = 0;

        int removableDisks = 0;

        while (driveMask != 0)
        {
            if (driveMask & 1)
            {
                wchar_t drivename[] = L"\\\\.\\A:\\";
                drivename[4] += i;

                if (checkDriveType(drivename))
                {
                    qDebug() << (QString::fromWCharArray(&drivename[4]));
                    emit foundDevice(0, 0, QString::fromWCharArray(&drivename[4]));
                    removableDisks++;
                }
            }
            driveMask >>= 1;
            ++i;
        }
        if (removableDisks == 0) {
            emit searcherMessage("No removable device found", true);
        } else {
            emit searcherMessage(QString("Found %1 storage device%2").arg(removableDisks).arg(removableDisks > 1 ? "s":""));
        }

}
