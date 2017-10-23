#ifndef LINUXSTORAGEDEVICE_H
#define LINUXSTORAGEDEVICE_H


#include "../StorageDevice.h"


class LinuxStorageDevice : public StorageDevice
{
public:
    LinuxStorageDevice(void);

    LinuxStorageDevice(int vid, int pid, int blockSize,
                       QString const& devicePath,
                       QVector<QString> const& mountpoints);

    ~LinuxStorageDevice(void) override;

    int vid(void) const override;

    int pid(void) const override;

    ExecutionStatus unmountAllMountpoints(void) override;

    ExecutionStatus unmount(QString const& mountpoint) override;

    long recommendedBlockSize(void) const override;

    ExecutionStatus open(int* const filedesc) override;

    ExecutionStatus close(void) override;

    QVector<QString> mountpoints(void) const override;

    QString diskPath(void) const override;

    QString toString(void) const override;

private:
    int _vid;
    int _pid;
    long _blockSize;
    QString _diskPath;
    QVector<QString> _mountpoints;
    int _fd;
};

#endif // LINUXSTORAGEDEVICE_H
