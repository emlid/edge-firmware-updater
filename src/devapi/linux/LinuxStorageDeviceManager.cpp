#include <libudev.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mount.h>
#include <unistd.h>

#include "LinuxStorageDeviceManager.h"
#include "LinuxStorageDevice.h"


LinuxStorageDeviceManager::LinuxStorageDeviceManager(void) { }


QVector<std::shared_ptr<StorageDevice>>
    LinuxStorageDeviceManager::physicalDrives(int vid, int pid)
{
    QVector<std::shared_ptr<StorageDevice>> storageDeviceList;

    std::unique_ptr<udev, decltype(&udev_unref)>
            manager(::udev_new(), &udev_unref);

    std::unique_ptr<udev_enumerate, decltype(&udev_enumerate_unref)>
            enumerate(::udev_enumerate_new(manager.get()), &udev_enumerate_unref);

    ::udev_enumerate_add_match_subsystem(enumerate.get(), "block");
    ::udev_enumerate_add_match_property(enumerate.get(), "DEVTYPE", "disk");
    ::udev_enumerate_scan_devices(enumerate.get());

    ::udev_list_entry* deviceList = ::udev_enumerate_get_list_entry(enumerate.get());
    ::udev_list_entry* entry = nullptr;

    auto toLowHex = "%04x";
    auto requiredVid = QString::asprintf(toLowHex, vid);
    auto requiredPid = QString::asprintf(toLowHex, pid);

    udev_list_entry_foreach(entry, deviceList) {
        auto path = ::udev_list_entry_get_name(entry);
        std::unique_ptr<udev_device, decltype(&udev_device_unref)>
                device(::udev_device_new_from_syspath(manager.get(), path), &udev_device_unref);

        if (device == nullptr) {
            continue;
        }

        auto deviceVid = QString(::udev_device_get_property_value(device.get(), "ID_VENDOR_ID"));
        auto devicePid = QString(::udev_device_get_property_value(device.get(), "ID_MODEL_ID"));

        qDebug() << "Vid :" << deviceVid << " " << requiredVid;
        qDebug() << "Pid :" << devicePid << " " << requiredPid;

        if (deviceVid != requiredVid) {
            continue;
        }

        auto diskPath = QString(::udev_device_get_devnode(device.get()));
        auto relatedMountpoints = _relatedMountpoints(diskPath);

        int blockSize = -1;
        _getRecommendedBlockSize(diskPath, &blockSize);

        auto removableDevice =
                new LinuxStorageDevice(vid, pid, blockSize, diskPath, relatedMountpoints);

        storageDeviceList.push_back(std::shared_ptr<StorageDevice>(removableDevice));
    }

    return storageDeviceList;
}


QVector<QString> LinuxStorageDeviceManager::mountpoints(void)
{
    auto mountedVolumes = QStorageInfo::mountedVolumes();
    QVector<QString> mntpoints;
    mntpoints.reserve(mountedVolumes.size());

    for (auto const& volume : mountedVolumes) {
        mntpoints.push_back(volume.device());
    }

    return mntpoints;
}


ExecutionStatus LinuxStorageDeviceManager::
    _getRecommendedBlockSize(QString const& diskPath, int* const blockSize)
{
    struct stat fileStat;

    if (::stat(diskPath.toStdString().data(), &fileStat)) {
        return ExecutionStatus(errno, diskPath + ": can not get stat structure.");
    }

    (*blockSize) = fileStat.st_blksize;

    return ExecutionStatus::SUCCESS;
}


QVector<QString> LinuxStorageDeviceManager::
    _relatedMountpoints(QString const& diskPath)
{
    QVector<QString> neededMountpoints;
    auto mntpoints = mountpoints();

    for (QString const& mntpt : mntpoints) {
        if (mntpoints.startsWith(diskPath)) {
            neededMountpoints.push_back(mntpt);
        }
    }

    return neededMountpoints;
}
