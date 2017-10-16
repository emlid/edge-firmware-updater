#ifndef WINDOWSSTORAGEDEVICESCANNER_H
#define WINDOWSSTORAGEDEVICESCANNER_H

#include <QObject>
#include <QObject>
#include <QtCore>

/* WinAPI headers */
#include <windows.h>
#include <SetupAPI.h>

#include "StorageDeviceManager.h"

class WindowsStorageDeviceManager : public StorageDeviceManager
{
    Q_OBJECT
public:
    using DeviceHandler_t = std::function<void(PSP_DEVICE_INTERFACE_DETAIL_DATA)>;

    WindowsStorageDeviceManager();

    QVector<std::shared_ptr<StorageDevice>> physicalDrives(int vid = -1, int pid = -1) override;

    QVector<QString> mountpoints(void);

    ExecutionStatus forEachDevices(GUID guid, DeviceHandler_t deviceHandler);

private:
    ExecutionStatus _getDriveNumbersRelatedWith(QString const& mountpoint, QVector<int>* const driveNumbers);

    ExecutionStatus _getRecommendedBlockSize(QString const& diskPath, long* const blockSize);

    ExecutionStatus _getPhysicalDriveNumber(QString const& device, int * const driveNumber);

    std::shared_ptr<StorageDevice> _constructStorageDevice(QString devicePath, int vid, int pid);

    QString _convertToMountpoint(QString volumeRootPath);

    QString _diskPathRelatedWithDevice(QString devicePath);

    QString _extractSerialNumber(QString devicePath);

    int _extractVid(QString devicePath);

    int _extractPid(QString devicePath);
};

#endif // WINDOWSSTORAGEDEVICESCANNER_H
