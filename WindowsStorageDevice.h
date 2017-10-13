#ifndef WINDOWSSTORAGEDEVICE_H
#define WINDOWSSTORAGEDEVICE_H

#include "StorageDevice.h"

class WindowsStorageDevice: public StorageDevice
{
public:
    WindowsStorageDevice();

    WindowsStorageDevice(int vid, int pid, int blockSize,
                         int driveNumber, QString const& devicePath,
                         QVector<QString> const& mountpoints);

    int vid(void) override;

    int pid(void) override;

    ulong recommendedBlockSize(void) override;

    ExitStatus open(int* const filedesc) override;

    ExitStatus unmountAllMountpoints(void) override;

    ExitStatus unmount(QString const& mountpoint) override;

    QVector<QString> mountpoints(void) override;

    QString diskPath(void) override;

    QString toString(void) override;

    int driveNumber(void);

    QString devicePath(void);

private:
    int _vid;
    int _pid;
    int _driveNumber;
    ulong _blockSize;
    QVector<QString> _mountpoints;
    QString _devicePath;
};

#endif // WINDOWSSTORAGEDEVICE_H
