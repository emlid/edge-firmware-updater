#include "../usbutil.h"
#include "RpiBootImpl.h"
#include <QtCore>

#if defined(Q_OS_WIN)
#pragma warning(push)
#pragma warning(disable:4200)
#include <libusb-1.0/libusb.h>
#pragma warning(pop)
#else
#include <libusb-1.0/libusb.h>
#endif


auto usb::makeRpiBoot(int vid, const QList<int> &pids)
    -> std::unique_ptr<usb::IRpiBoot>
{
    return std::make_unique<usb::RpiBootImpl>(vid, pids);
}


auto usb::countOfDevicesWith(int vid, QList<int> const& pids) -> int
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

        if (desc.idVendor == vid && pids.contains(desc.idProduct)) {
           bootable++;
        }
    }

    libusb_exit(context);
    return bootable;
}
