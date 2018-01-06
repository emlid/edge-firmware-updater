#include <libudev.h>
#include <blkid/blkid.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "StorageDeviceInfo.h"
#include "util.h"

using namespace devlib;

class impl::StorageDeviceInfo_Private
{
public:
    StorageDeviceInfo_Private(int vid, int pid, QString const& deviceFilePath)
        : _vid(vid), _pid(pid), _deviceFilePath(deviceFilePath)
    { }

    int partitionsCount(void) const
    {
        auto asStdString = _deviceFilePath.toStdString();
        auto probe = ::blkid_new_probe_from_filename(asStdString.data());

        if (!probe) {
            qCritical() << "Failed to create blkid probe for device: " << _deviceFilePath;
            return -1;
        }

        auto partlist = ::blkid_probe_get_partitions(probe);

        if (!partlist) {
            qWarning() << "Device doesn't have any partitions.";
            return 0;
        }

        auto partCount = ::blkid_partlist_numof_partitions(partlist);

        ::blkid_free_probe(probe);
        return partCount;
    }

    int _vid;
    int _pid;
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
    Q_ASSERT(_pimpl);
    auto info = QString();
    QTextStream infoStream(&info);

    auto collectInfo = [] (auto&& elems) {
        auto info = QString();
        QTextStream stream(&info);

        stream << fmt::tab() << fmt::beg() << endl;
        for (auto& elem : elems) {
            stream << fmt::tab() << fmt::tab() << elem.info() << endl;
        }
        stream << fmt::tab() << fmt::end();

        return info;
    };

    infoStream
            << "Device "  <<  fmt::beg() << endl
            << fmt::tab() << "vid: " << vid() << endl
            << fmt::tab() << "pid: " << pid() << endl
            << fmt::tab() << "name: " << filePath() << endl
            << fmt::tab() << "parts: " << collectInfo(partitions()) << endl
            << fmt::tab() << "mntpts: " << collectInfo(mountpoints()) << endl
            << fmt::end();

    return info;
}


QString StorageDeviceInfo::filePath(void) const noexcept
{
    Q_ASSERT(_pimpl);
    return _pimpl->_deviceFilePath;
}


QList<Partition> StorageDeviceInfo::partitions(void) const
{
    Q_ASSERT(_pimpl);

    auto partitions = QList<Partition>();
    auto partsCount = _pimpl->partitionsCount();

    if (partsCount <= 0) {
        qWarning() << "No partitions found for: " << _pimpl->_deviceFilePath;
        return partitions;
    }

    auto valueOf =[] (auto& param, auto& probe, auto buffer) {
        ::blkid_probe_lookup_value(probe, param, &buffer, nullptr);
        return buffer == nullptr ?
            QString("") : QString(buffer);
    };

    for (auto i = 1; i <= partsCount; i++) {
        auto partName = _pimpl->_deviceFilePath;
        partName.append(QString::number(i));

        auto probe = ::blkid_new_probe_from_filename(partName.toStdString().data());

        if (!probe) {
            qWarning() << "Failed to create blkid "
                          "probe for part name: " << partName;
            continue;
        }

        char const* buffer = nullptr;

        ::blkid_do_probe(probe);
        auto partLabel = valueOf("LABEL", probe, buffer);
        auto fstype    = valueOf("TYPE", probe, buffer);
        ::blkid_free_probe(probe);

        if (partLabel.isEmpty()) {
            qWarning() << "This partition hasn't label: "
                       << partName;
        } else {
            qInfo() << "Partition: "  << partName
                    << " with label: " << partLabel
                    << " found.";
        }

        partitions.push_back(Partition(partName, fstype, partLabel));
    }

    return partitions;
}


QList<Mountpoint> StorageDeviceInfo::mountpoints(void) const
{
    Q_ASSERT(_pimpl);
    QList<Mountpoint> neededMountpoints;
    auto mountedVolumes = QStorageInfo::mountedVolumes();

    for (auto const& mntpt : mountedVolumes) {
        if (QString(mntpt.device()).startsWith(_pimpl->_deviceFilePath)) {
            neededMountpoints.push_back(Mountpoint(mntpt.rootPath()));
        }
    }

    return neededMountpoints;
}


QList<StorageDeviceInfo> StorageDeviceInfo::availableDevices(void)
{
    auto storageDeviceList = QList<StorageDeviceInfo>();

    std::unique_ptr<udev, decltype(&udev_unref)>
            manager(::udev_new(), &udev_unref);

    std::unique_ptr<udev_enumerate, decltype(&udev_enumerate_unref)>
            enumerate(::udev_enumerate_new(manager.get()), &udev_enumerate_unref);

    ::udev_enumerate_add_match_subsystem(enumerate.get(), "block");
    ::udev_enumerate_add_match_property(enumerate.get(), "DEVTYPE", "disk");
    ::udev_enumerate_scan_devices(enumerate.get());

    ::udev_list_entry* deviceList = ::udev_enumerate_get_list_entry(enumerate.get());
    ::udev_list_entry* entry = nullptr;

    udev_list_entry_foreach(entry, deviceList) {
        auto path = ::udev_list_entry_get_name(entry);
        std::unique_ptr<udev_device, decltype(&udev_device_unref)>
                device(::udev_device_new_from_syspath(manager.get(), path), &udev_device_unref);

        if (device == nullptr) {
            continue;
        }

        auto deviceVid = QString(::udev_device_get_property_value(device.get(), "ID_VENDOR_ID"));
        auto devicePid = QString(::udev_device_get_property_value(device.get(), "ID_MODEL_ID"));
        auto diskPath  = QString(::udev_device_get_devnode(device.get()));

        auto base = 16;
        auto storageDevInfo = StorageDeviceInfo(deviceVid.toInt(nullptr, base),
                                                devicePid.toInt(nullptr, base),
                                                diskPath);
        storageDeviceList.push_back(storageDevInfo);
    }

    return storageDeviceList;
}
