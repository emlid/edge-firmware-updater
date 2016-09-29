#include "firmwareupgradecontroller.h"
#include "rpiboot.h"
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "libusb.h"
#include <QDebug>


void DeviceSearcher::startFindBoardLoop()
{
    emit searcherMessage("Scan for devices...");

    int bootable = findBootableDevices();
    qDebug() << "bootable:" << bootable;

    if (bootable > 0) {
        /*
         *  run rpiboot (not implemented yet)
         */
    }

    /*
     *  enumerateDevices();
     */

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
    /*
     *  functionality is not implemented yet
     */
}
