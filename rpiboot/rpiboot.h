#ifndef USBBOOTRPI_H
#define USBBOOTRPI_H

#include <QThread>

void rpiboot(void);

void startRpiBoot(QThread *, int bootableDevces);

#endif // USBBOOTRPI_H


