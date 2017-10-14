/* WinAPI headers */
#include <initguid.h>
#include <tchar.h>
#include <usbiodef.h>

#include "WindowsStorageDeviceManager.h"
#include "WindowsStorageDevice.h"


WindowsStorageDeviceManager::WindowsStorageDeviceManager() {}


std::shared_ptr<StorageDevice> WindowsStorageDeviceManager::
    _constructStorageDevice(QString devicePath, int vid, int pid)
{
    QString diskPath = _diskPathRelatedWithDevice(devicePath);
    int driveNumber = 0;
    if (_getPhysicalDriveNumber(diskPath, &driveNumber).failed()) {
        return std::shared_ptr<StorageDevice>(nullptr);
    }

    ulong blockSize = 0;
    if (_getRecommendedBlockSize(QString("\\\\.\\PhysicalDrive%1").arg(driveNumber), &blockSize).failed()) {
        blockSize = -1;
    }

    auto mntpoints = mountpoints();
    QVector<QString> relatedMountpoints;

    for (QString mntpoint : mntpoints) {
        QVector<int> mountpointDriveNumbers;
        if (_getDriveNumbersRelatedWith(mntpoint, &mountpointDriveNumbers).failed()) {
            continue;
        }

        if (mountpointDriveNumbers.contains(driveNumber)) {
           relatedMountpoints.push_back(mntpoint);
        }
    }

    return std::shared_ptr<StorageDevice>(
        new WindowsStorageDevice(vid, pid, blockSize, driveNumber, diskPath, relatedMountpoints)
    );
}


QVector<std::shared_ptr<StorageDevice>> WindowsStorageDeviceManager::
    physicalDrives(int vid, int pid)
{
    QVector<std::shared_ptr<StorageDevice>> devicesList;

    forEachDevices(GUID_DEVINTERFACE_USB_DEVICE,
        [&devicesList, vid, pid, this] (PSP_DEVICE_INTERFACE_DETAIL_DATA detailData) -> void {
            QString devicePath = QString::fromWCharArray(detailData->DevicePath);

            auto devVid = _extractVid(devicePath);
            auto devPid = _extractPid(devicePath);

            if (devVid != vid || devPid != pid) return;

            auto storageDevicePtr = _constructStorageDevice(devicePath, vid, pid);
            if (storageDevicePtr.get() != nullptr) {
                devicesList.push_back(storageDevicePtr);
            }
        }
    );

    return devicesList;
}


QVector<QString> WindowsStorageDeviceManager::mountpoints()
{
    auto volumes = QStorageInfo::mountedVolumes();
    QVector<QString> mountpoints;
    mountpoints.reserve(volumes.size());

    for (QStorageInfo vol : volumes) {
        mountpoints.push_back(
            _convertToMountpoint(vol.rootPath())
        );
    }

    return mountpoints;
}


ExecutionStatus WindowsStorageDeviceManager::
    _getDriveNumbersRelatedWith(QString const& mountpoint, QVector<int>* const driveNumbers)
{
    auto volumeHandle = ::CreateFile(mountpoint.toStdWString().data(),
                                     GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL,
                                     OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

    if (volumeHandle == INVALID_HANDLE_VALUE) {
        return ExecutionStatus(::GetLastError(), mountpoint + ": Getting volume handle failed.");
    }

    VOLUME_DISK_EXTENTS diskExtents;

    int bytesReturned = 0;
    auto successful = ::DeviceIoControl(volumeHandle, IOCTL_VOLUME_GET_VOLUME_DISK_EXTENTS,
                                        NULL, 0, &diskExtents, sizeof(diskExtents),
                                        (LPDWORD)&bytesReturned, NULL);

    if (!successful) {
       ::CloseHandle(volumeHandle);
       return ExecutionStatus(::GetLastError(), mountpoint + ": Getting disk extents failed.");
    }

    driveNumbers->reserve(diskExtents.NumberOfDiskExtents);

    for (auto i = 0u; i < diskExtents.NumberOfDiskExtents; i++) {
       driveNumbers->push_back(diskExtents.Extents[i].DiskNumber);
    }

    ::CloseHandle(volumeHandle);

    return ExecutionStatus::SUCCESS;
}


ExecutionStatus WindowsStorageDeviceManager::
    _getRecommendedBlockSize(QString const& diskPath, ulong* const blockSize)
{
    auto handle = ::CreateFile(diskPath.toStdWString().data(), 0,
                               FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);

    if (handle == INVALID_HANDLE_VALUE) {
        return ExecutionStatus(::GetLastError(), diskPath + ": cant open for getting disk info");
    }


    DWORD junk;
    DISK_GEOMETRY_EX diskgeometry;
    auto successful = ::DeviceIoControl(handle, IOCTL_DISK_GET_DRIVE_GEOMETRY_EX,
                                        NULL, 0, &diskgeometry, sizeof(diskgeometry), &junk, NULL);

    if (!successful) {
        ::CloseHandle(handle);
        return ExecutionStatus(::GetLastError(), diskPath + ": cant get disk info");
    }

    (*blockSize) = diskgeometry.Geometry.BytesPerSector * 4;

    ::CloseHandle(handle);

    return ExecutionStatus::SUCCESS;
}


QString WindowsStorageDeviceManager::
    _convertToMountpoint(QString volumeRootPath)
{
    return volumeRootPath.prepend("\\\\.\\").replace('/', "");
}


ExecutionStatus WindowsStorageDeviceManager::
    _getPhysicalDriveNumber(QString const& physicalDriveName, int* const driveNumber)
{
    STORAGE_DEVICE_NUMBER deviceNumber = {0};

    auto diskHandle = ::CreateFile(physicalDriveName.toStdWString().data(),
                                   0, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL,
                                   OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

    if (diskHandle == INVALID_HANDLE_VALUE) {
        return ExecutionStatus(::GetLastError(),
                          "Physical Drive: " + physicalDriveName + " Create file failed.");
    }

    int bytesReturned = 0;
    auto successful = ::DeviceIoControl(diskHandle, IOCTL_STORAGE_GET_DEVICE_NUMBER,
                                        NULL, 0, &deviceNumber, sizeof(deviceNumber),
                                        (LPDWORD)&bytesReturned, NULL);

    if (!successful) {
       ::CloseHandle(diskHandle);
       return ExecutionStatus(::GetLastError(),
                         "Physical Drive: " + physicalDriveName + " Getting device number failed.");
    }

    CloseHandle(diskHandle);

    (*driveNumber) = deviceNumber.DeviceNumber;

    return ExecutionStatus::SUCCESS;
}


ExecutionStatus WindowsStorageDeviceManager::
    forEachDevices(GUID guid, WindowsStorageDeviceManager::DeviceHandler_t deviceHandler)
{
    SP_DEVINFO_DATA devInfoData;
    devInfoData.cbSize= sizeof(SP_DEVINFO_DATA);

    SP_DEVICE_INTERFACE_DATA deviceIntData;
    deviceIntData.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);

    HDEVINFO deviceInfoSet =
            ::SetupDiGetClassDevs(&guid, NULL, NULL, DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);

    if (deviceInfoSet == INVALID_HANDLE_VALUE) {
        return ExecutionStatus(::GetLastError(), "Getting device info set failed.", true);
    }

    QVector<char> buffer;

    for (int i = 0; ::SetupDiEnumDeviceInterfaces(deviceInfoSet, NULL, &guid, i, /*out*/&deviceIntData); i++) {
        PSP_DEVICE_INTERFACE_DETAIL_DATA detailData;
        int detailDataSize = 0;

        // get required size of detailData
        bool successful = ::SetupDiGetDeviceInterfaceDetail(deviceInfoSet, &deviceIntData,
                                                            NULL, 0, /*out*/PDWORD(&detailDataSize), NULL);

        if (detailDataSize == 0) {
            qDebug() << "availableDevices: get required size failed.";
            continue;
        }

        if (buffer.capacity() < detailDataSize) {
            buffer.reserve(detailDataSize);
        }

        detailData = reinterpret_cast<PSP_DEVICE_INTERFACE_DETAIL_DATA>(buffer.data());
        ::ZeroMemory(detailData, detailDataSize);
        detailData->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);

        // get detailData
        successful = ::SetupDiGetDeviceInterfaceDetail(deviceInfoSet, &deviceIntData,
                                                     /*out*/detailData, detailDataSize, NULL, NULL);

        if (!successful) {
            qDebug() << "availableDevices: get detailData failed.";
            continue;
        }

        deviceHandler(detailData);
    }

    ::SetupDiDestroyDeviceInfoList(deviceInfoSet);

    return ExecutionStatus::SUCCESS;
}


QString WindowsStorageDeviceManager::_diskPathRelatedWithDevice(QString devicePath)
{
    QString usbDeviceSerialNumber = _extractSerialNumber(devicePath);
    QString diskPath("");


    forEachDevices(GUID_DEVINTERFACE_DISK,
        [&diskPath, &usbDeviceSerialNumber] (PSP_DEVICE_INTERFACE_DETAIL_DATA detailData) -> void {
            auto path = QString::fromWCharArray(detailData->DevicePath);
            if (!path.contains(usbDeviceSerialNumber)) return;

            diskPath = std::move(path);
        }
    );

    return diskPath;
}


QString WindowsStorageDeviceManager::_extractSerialNumber(QString devicePath)
{
    auto serial = QString(devicePath).replace(QRegularExpression(".*#(.*)#{.*"), "\\1");
    return serial;
}


int WindowsStorageDeviceManager::_extractVid(QString devicePath)
{
    return QString(devicePath)
        .replace(QRegularExpression(".*vid_(.{4}).*"), "0x\\1").toInt(Q_NULLPTR, 16);
}


int WindowsStorageDeviceManager::_extractPid(QString devicePath)
{
    return QString(devicePath)
        .replace(QRegularExpression(".*pid_(.{4}).*"), "0x\\1").toInt(Q_NULLPTR, 16);
}
