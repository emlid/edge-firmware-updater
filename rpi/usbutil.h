#ifndef USBUTIL_H
#define USBUTIL_H

#include <QList>
#include <memory>
#include "RpiBoot.h"

namespace usb {
    auto countOfDevicesWith(int vid, QList<int> const& pids)-> int;
    auto makeRpiBoot(int vid, QList<int> const& pids) -> std::unique_ptr<usb::IRpiBoot>;

}


#endif // USBUTIL_H
