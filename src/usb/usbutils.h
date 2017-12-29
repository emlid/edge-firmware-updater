#ifndef USBUTILS_H
#define USBUTILS_H

#include <QtCore>

namespace usb {
    int countOfDevicesWith(int vid, QList<int> const& pids);
}

#endif // USBUTILS_H
