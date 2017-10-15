#ifndef LINUXSTORAGEDEVICEMANAGER_H
#define LINUXSTORAGEDEVICEMANAGER_H


#include "StorageDeviceManager.h"


class LinuxStorageDeviceManager : public StorageDeviceManager
{
public:
    LinuxStorageDeviceManager(void);

    QVector<std::shared_ptr<StorageDevice>> physicalDrives(int vid = -1, int pid = -1) override;

    QVector<QString> mountpoints(void);
private:
    ExecutionStatus _getRecommendedBlockSize(QString const& diskPath, int* const blockSize);
};

#endif // LINUXSTORAGEDEVICEMANAGER_H
