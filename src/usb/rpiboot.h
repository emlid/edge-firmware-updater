#ifndef USBBOOTRPI_H
#define USBBOOTRPI_H

#include <QtCore>

namespace usb {
    class RpiBoot;
}

class RpiBootPrivate;

class usb::RpiBoot
{
public:
    RpiBoot(int vid, QList<int> const& pid);
    ~RpiBoot(void);

    int bootAsMassStorage(void);
private:
    RpiBootPrivate* _pimpl;
};


#endif // USBBOOTRPI_H
