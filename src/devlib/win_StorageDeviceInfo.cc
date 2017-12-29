#include <initguid.h>
#include <tchar.h>
#include <usbiodef.h>
#include <windows.h>
#include <SetupAPI.h>

#include "StorageDeviceInfo.h"

using namespace devlib;

class impl::StorageDeviceInfo_Private {
public:
    StorageDeviceInfo_Private(int vid, int pid, QString const& deviceFilePath)
        : _vid(vid), _pid(pid), _deviceFilePath(deviceFilePath)
    { }

    struct DevInfo {
        DevInfo(int in_vid, int in_pid)
            : vid(in_vid), pid(in_pid)
        { }

        int vid, pid;
    };

    using DeviceHandler = std::function<void(PSP_DEVICE_INTERFACE_DETAIL_DATA)>;

    static bool foreachDevices(GUID guid, DeviceHandler deviceHandler) {
        SP_DEVINFO_DATA devInfoData;
        devInfoData.cbSize = sizeof(SP_DEVINFO_DATA);

        SP_DEVICE_INTERFACE_DATA deviceIntData;
        deviceIntData.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);

        HDEVINFO deviceInfoSet =
            ::SetupDiGetClassDevs(&guid, NULL, NULL, DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);

        if (deviceInfoSet == INVALID_HANDLE_VALUE) {
            return false;
        }

        QVector<char> buffer;

        for (int i = 0;
             ::SetupDiEnumDeviceInterfaces(deviceInfoSet, NULL, &guid, i, &deviceIntData);
             i++)
        {
            PSP_DEVICE_INTERFACE_DETAIL_DATA detailData;
            int detailDataSize = 0;

            // get required size of detailData
            bool successful = ::SetupDiGetDeviceInterfaceDetail(deviceInfoSet, &deviceIntData,
                                                            NULL, 0, /*out*/PDWORD(&detailDataSize), NULL);

            if (detailDataSize == 0) {
                qDebug() << "forEachDevices: get required size failed.";
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

        return true;
    }

    static DevInfo extractDevInfo(QString const& devicePath) {
        auto value = [&devicePath] (QString const& key) {
            return QString(devicePath)
                  .replace(QRegularExpression(".*" + key + "_(.{4}).*"), "0x\\1")
                  .toInt(Q_NULLPTR, 16);
        };

        return {value("vid"), value("pid")};
    }

    static QString extractSerialNumber(QString const& devicePath) {
        return QString(devicePath)
                .replace(QRegularExpression(".*#(.*)#{.*"), "\\1");
    }

    static QString deviceDiskPath(QString const& devicePath) {

        QString usbDeviceSerialNumber = extractSerialNumber(devicePath);
        QString deviceDiskPath("");

        foreachDevices(GUID_DEVINTERFACE_DISK,
            [&deviceDiskPath, &usbDeviceSerialNumber]
                (PSP_DEVICE_INTERFACE_DETAIL_DATA detailData) -> void {
                    auto path = QString::fromWCharArray(detailData->DevicePath);
                    if (!path.contains(usbDeviceSerialNumber)) return;

                    deviceDiskPath = std::move(path);
                }
        );

        return deviceDiskPath;
    }

    static QVector<int> driveNumbersMountedTo(QString const& mountpoint)
    {
        auto driveNumbers = QVector<int>();
        auto volumeHandle = ::CreateFile(mountpoint.toStdWString().data(),
                                         GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL,
                                         OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

        if (volumeHandle == INVALID_HANDLE_VALUE) {
            return driveNumbers;
        }

        VOLUME_DISK_EXTENTS diskExtents;

        int bytesReturned = 0;
        auto successful = ::DeviceIoControl(volumeHandle, IOCTL_VOLUME_GET_VOLUME_DISK_EXTENTS,
                                            NULL, 0, &diskExtents, sizeof(diskExtents),
                                            (LPDWORD)&bytesReturned, NULL);

        if (!successful) {
           ::CloseHandle(volumeHandle);
            return driveNumbers;
        }

        driveNumbers.reserve(diskExtents.NumberOfDiskExtents);

        for (auto i = 0u; i < diskExtents.NumberOfDiskExtents; i++) {
           driveNumbers.push_back(diskExtents.Extents[i].DiskNumber);
        }

        ::CloseHandle(volumeHandle);

        return driveNumbers;
    }

    static int driveNumber(QString const& physicalDriveName)
    {
        STORAGE_DEVICE_NUMBER deviceNumber = {0};

        auto diskHandle = ::CreateFile(physicalDriveName.toStdWString().data(),
                                       0, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL,
                                       OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

        if (diskHandle == INVALID_HANDLE_VALUE) {
            return -1;
        }

        int bytesReturned = 0;
        auto successful = ::DeviceIoControl(diskHandle, IOCTL_STORAGE_GET_DEVICE_NUMBER,
                                            NULL, 0, &deviceNumber, sizeof(deviceNumber),
                                            (LPDWORD)&bytesReturned, NULL);

        if (!successful) {
           ::CloseHandle(diskHandle);
            return -1;
        }

        ::CloseHandle(diskHandle);

        return deviceNumber.DeviceNumber;
    }

    int _pid;
    int _vid;
    QString _deviceFilePath;
};



StorageDeviceInfo::StorageDeviceInfo(int vid, int pid, QString const& devFilePath)
    : _pimpl(new impl::StorageDeviceInfo_Private(vid, pid, devFilePath))
{ }


StorageDeviceInfo::StorageDeviceInfo(StorageDeviceInfo&& info) noexcept
    : _pimpl(std::move(info._pimpl))
{ }


StorageDeviceInfo::StorageDeviceInfo(StorageDeviceInfo const& info)
    : _pimpl(new impl::StorageDeviceInfo_Private(*info._pimpl))
{ }


StorageDeviceInfo& StorageDeviceInfo::operator =(StorageDeviceInfo&& info) noexcept
{
    _pimpl = std::move(info._pimpl);
    return *this;
}


StorageDeviceInfo& StorageDeviceInfo::
    operator =(StorageDeviceInfo const& info)
{
    _pimpl.reset(new impl::StorageDeviceInfo_Private(*info._pimpl));
    return *this;
}


StorageDeviceInfo::~StorageDeviceInfo(void)
{ }


int StorageDeviceInfo::vid(void) const noexcept
{
    Q_ASSERT(_pimpl);
    return _pimpl->_vid;
}


int StorageDeviceInfo::pid(void) const noexcept
{
    Q_ASSERT(_pimpl);
    return _pimpl->_pid;
}


QString StorageDeviceInfo::info(void) const noexcept
{
    return "";
}


QString StorageDeviceInfo::filePath(void) const noexcept
{
    Q_ASSERT(_pimpl);
    return _pimpl->_deviceFilePath;
}


QList<Partition> StorageDeviceInfo::partitions(void) const
{
    Q_ASSERT(_pimpl);
    return {};
}


QList<Mountpoint> StorageDeviceInfo::mountpoints(void) const
{
    auto toMountpointPath = [] (QString const& volumeRootPath) -> QString {
        return QString(volumeRootPath).prepend("\\\\.\\").replace('/', "");
    };

    auto volumes = QStorageInfo::mountedVolumes();
    auto mountpoints = QList<Mountpoint>();

    auto driveNumber = QString(_pimpl->_deviceFilePath)
            .replace("\\\\.\\PhysicalDrive", "").toInt();

    for (QStorageInfo vol : volumes) {
        auto mntptPath = toMountpointPath(vol.rootPath());
        auto driveNums = impl::StorageDeviceInfo_Private::
                driveNumbersMountedTo(mntptPath);

        if (driveNums.contains(driveNumber)) {
            qInfo() << "found device mountpoint: " << mntptPath;
            mountpoints.push_back({mntptPath});
        }
    }

    return mountpoints;
}



QList<StorageDeviceInfo> StorageDeviceInfo::availableDevices(void)
{
    using PrivateInfo = impl::StorageDeviceInfo_Private;
    auto devicesList = QList<StorageDeviceInfo>();

    PrivateInfo::foreachDevices(GUID_DEVINTERFACE_USB_DEVICE,
        [&devicesList] (PSP_DEVICE_INTERFACE_DETAIL_DATA detailData) -> void {
            QString devicePath = QString::fromWCharArray(detailData->DevicePath);

            auto devInfo        = PrivateInfo::extractDevInfo(devicePath);
            auto deviceDiskPath = PrivateInfo::deviceDiskPath(devicePath);
            auto driveNumber    = PrivateInfo::driveNumber(deviceDiskPath);

            if (driveNumber == -1) {
                return;
            }

            auto deviceFilePath = QString("\\\\.\\PhysicalDrive%1").arg(driveNumber);

            devicesList.push_back({devInfo.vid, devInfo.pid, deviceFilePath});
        }
    );

    return devicesList;
}
