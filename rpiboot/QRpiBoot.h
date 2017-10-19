#ifndef QRPIBOOT_H
#define QRPIBOOT_H

#include <QThread>

namespace rpiboot {
    void rpiboot(void);

    void startRpiBoot(QThread* thread, int bootableDevices);
}


#endif // QRPIBOOT_H
