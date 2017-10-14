#ifndef WINDOWSSTORAGEDEVICE_H
#define WINDOWSSTORAGEDEVICE_H

#include "StorageDevice.h"
#include <windows.h>

class WindowsStorageDevice: public StorageDevice
{
public:
    WindowsStorageDevice(void);

    WindowsStorageDevice(int vid, int pid, int blockSize,
                         int driveNumber, QString const& devicePath,
                         QVector<QString> const& mountpoints);

    ~WindowsStorageDevice(void) override;

    int vid(void) const override;

    int pid(void) const override;

    ulong recommendedBlockSize(void) const override;

    ExitStatus open(int* const filedesc) override;

    ExitStatus close(void) override;

    ExitStatus unmountAllMountpoints(void) override;

    ExitStatus unmount(QString const& mountpoint) override;

    ExitStatus remountAllMountpoints(void);

    ExitStatus remount(QString const& mountpoint);

    QVector<QString> mountpoints(void) const override;

    QString diskPath(void) const override;

    QString toString(void) const override;

    int driveNumber(void) const;

    QString devicePath(void) const;

private:
    int _vid;
    int _pid;
    int _driveNumber;
    HANDLE _handle;
    ulong _blockSize;
    QVector<QString> _mountpoints;
    QMap<QString, HANDLE> _unmountedMountpoints;
    QString _devicePath;
};

#endif // WINDOWSSTORAGEDEVICE_H
