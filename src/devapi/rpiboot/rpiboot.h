#ifndef USBBOOTRPI_H
#define USBBOOTRPI_H

#include <QtCore>


class RpiBootPrivate;


class RpiBoot
{
public:
    RpiBoot(int vid, QList<int> const& pid);

    int rpiDevicesCount(void) const;

    int bootAsMassStorage(void);

    ~RpiBoot(void);

private:
    /* Pointer to implementation */
    RpiBootPrivate* _pimpl;
};


#endif // USBBOOTRPI_H
