#include <libudev.h>
#include <sys/types.h>
#include <sys/stat.h>
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

    int const radix = 16;
    auto requiredVid = QString::number(vid, radix);
    auto requiredPid = QString::number(pid, radix);

    udev_list_entry_foreach(entry, deviceList) {
        auto path = ::udev_list_entry_get_name(entry);
        std::unique_ptr<udev_device, decltype(&udev_device_unref)>
                device(::udev_device_new_from_syspath(manager.get(), path), &udev_device_unref);

        if (device == nullptr) {
            continue;
        }

        auto deviceVid = QString(::udev_device_get_property_value(device.get(), "ID_VENDOR_ID"));
        auto devicePid = QString(::udev_device_get_property_value(device.get(), "ID_MODEL_ID"));

        if (deviceVid != requiredVid || devicePid != requiredPid) {
            continue;
        }

        auto diskPath = QString(::udev_device_get_devnode(device.get()));

        storageDeviceList.push_back(
            std::shared_ptr<StorageDevice>(
                new LinuxStorageDevice(vid, pid, 512, diskPath, QVector<QString>())
            )
        );
    }

    return storageDeviceList;
}


QVector<QString> LinuxStorageDeviceManager::mountpoints(void)
{
    return QVector<QString>();
}


ExecutionStatus LinuxStorageDeviceManager::
    _getRecommendedBlockSize(QString const& diskPath, int* const blockSize)
{
    struct stat fileStat;
    if (::stat(diskPath.toStdString().data(), &fileStat)) {
        return ExecutionStatus(errno, diskPath + ": can not get stat structure.");
    }

    (*blockSize) = stat.st_blksize;

    return ExecutionStatus::SUCCESS;
}
