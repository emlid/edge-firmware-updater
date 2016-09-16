#include "libusb-1.0/libusb.h"
#include <libudev.h>

#include "firmwareupgradecontroller.h"
#include "rpiboot.h"
#include "mainwindow.h"
#include "ui_mainwindow.h"

void DeviceSearcher::startFindBoardLoop()
{
    emit searcherMessage("Scan for devices...");

    int bootable = findBootableDevices();

    if (bootable > 0) {

        QThread rpibootThread;
        emit searcherMessage(QString("Found %1 bootable device%2").arg(bootable).arg(bootable > 1 ? "s":""));
        emit searcherMessage("Rpiboot started");
        startRpiBoot(&rpibootThread);

        startUdevMonitor(bootable);
        rpibootThread.quit();
    }

    enumerateDevices();

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

    struct udev* udev = udev_new();
    struct udev_device* dev;
    struct udev_monitor* mon;
    int fd;
    struct timeval tv;
    int ret;
    int catched = 0;

    mon = udev_monitor_new_from_netlink(udev, "udev");
    udev_monitor_filter_add_match_subsystem_devtype(mon, "block", "disk");
    udev_monitor_enable_receiving(mon);
    fd = udev_monitor_get_fd(mon);

    while (1) {
        fd_set fds;

        FD_ZERO(&fds);
        FD_SET(fd, &fds);
        tv.tv_sec = 0;
        tv.tv_usec = 0;

        ret = select(fd+1, &fds, NULL, NULL, &tv);

        if (ret > 0 && FD_ISSET(fd, &fds)) {
            dev = udev_monitor_receive_device(mon);
            if (!strcmp( udev_device_get_action(dev), "add") && !strcmp( udev_device_get_property_value(dev, "ID_VENDOR_ID"), VENDOR_ID)) {
                catched++;
                if (catched == count)
                {
                    break;
                }

            }
        }
    }

    udev_device_unref(dev);
    udev_unref(udev);
}


void DeviceSearcher::enumerateDevices()
{
    struct udev *udev = udev_new();
    struct udev_enumerate* enumerate;
    struct udev_list_entry *devices;
    struct udev_list_entry *entry;
    const char* path;
    struct udev_device* block;
    bool noDevice = true;

    enumerate = udev_enumerate_new(udev);

    udev_enumerate_add_match_subsystem(enumerate, "block");
    udev_enumerate_add_match_property(enumerate, "DEVTYPE", "disk");
    udev_enumerate_scan_devices(enumerate);
    devices = udev_enumerate_get_list_entry(enumerate);

    udev_list_entry_foreach(entry, devices) {
        path = udev_list_entry_get_name(entry);
        block = udev_device_new_from_syspath(udev, path);

        if (block) {
            const char* vendorID = udev_device_get_property_value(block, "ID_VENDOR_ID");
            if (vendorID != NULL && strcmp(vendorID, VENDOR_ID) == 0) {
                const char* modelID = udev_device_get_property_value(block, "ID_MODEL_ID");
                const char* devnode = udev_device_get_devnode(block);

                bool ok;
                uint32_t v = QString(vendorID).toUInt(&ok, 16);
                uint32_t p = QString(modelID).toUInt(&ok, 16);
                QString n = QString(devnode);
                emit foundDevice(v, p, n);

                noDevice = false;
                udev_device_unref(block);
                break;
            }
            udev_device_unref(block);
        }
    }

    udev_enumerate_unref(enumerate);
    udev_unref(udev);
    if (noDevice) {
        emit searcherMessage("No device found", true);
    } else {
        emit searcherMessage("Search finished successfully");
    }
}
