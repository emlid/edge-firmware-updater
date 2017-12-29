#include "edge.h"
#include "usb/usb.h"

/// functions

bool edge::available(void)
{
    return usb::countOfDevicesWith(edge::vid(), {edge::pid()}) > 0;
}


bool edge::isEdgeAsMassStorage(devlib::StorageDeviceInfo const& device) {
    return device.vid() == vid()
        && device.pid() == pidAsMassStorage();
}


util::Optional<edge::Initializer> edge::connect(void)
{
    if (edge::available()) {
        auto function = std::function<decltype(edge::impl::initialize)>
                (edge::impl::initialize);
        return function;
    }

    return {};
}

/// Initializer

util::Optional<edge::Device> edge::impl::initialize(void)
{
    auto rpiboot = usb::RpiBoot(edge::vid(), {edge::pid()});

    qInfo() << "Run rpiboot... waiting";

    if (rpiboot.bootAsMassStorage() == -1) {
        qWarning() << "Timeout: rpiboot failed";
        return {};
    }

    auto const maxPollingTime = 5000;
    auto const sleepTime      = 10;
    auto totalPollingTime     = 0;

    // We need to wait until OS detect our device, which was booted as mass storage
    qInfo() << "Search edge as mass storage... waiting";

    do {
        QThread::msleep(sleepTime);
        auto storageDevices = devlib::StorageDeviceInfo::availableDevices();
        auto edgePtr        = std::find_if(storageDevices.cbegin(),
                                           storageDevices.cend(),
                                           edge::isEdgeAsMassStorage);

        if (edgePtr != storageDevices.cend()) {
            return edge::Device(*edgePtr);
        }

        totalPollingTime += sleepTime;
    } while (totalPollingTime < maxPollingTime);

    qWarning() << "timeout: edge device not found";
    return {};
}


/// Device

edge::Device::Device(devlib::StorageDeviceInfo const& device)
    : _deviceInfo(new devlib::StorageDeviceInfo(device))
{ }


edge::Device::Device(void)
    : _deviceInfo(nullptr)
{ }


edge::Device::Device(Device&& device) noexcept
    : _deviceInfo(std::move(device._deviceInfo))
{ }


edge::Device::Device(Device const& device)
    : _deviceInfo(new devlib::StorageDeviceInfo(*device._deviceInfo))
{ }


edge::Device& edge::Device::operator =(Device&& device) noexcept
{
    _deviceInfo = std::move(device._deviceInfo);
    return *this;
}


edge::Device& edge::Device::operator =(Device const& device)
{
    _deviceInfo.reset(new devlib::StorageDeviceInfo(*device._deviceInfo));
    return *this;
}


bool edge::Device::stillAvailable(void) const
{
    Q_ASSERT(_deviceInfo);
    auto devices = devlib::StorageDeviceInfo::availableDevices();
    return std::any_of(devices.cbegin(), devices.cend(), edge::isEdgeAsMassStorage);
}


QString edge::Device::firmwareVersion(void)
{
    Q_ASSERT(_deviceInfo);
    auto fwVersion = QString("Undefined");
#ifdef Q_OS_WIN
    qInfo() << "Extract edge version";
    qInfo() << "Search volumes";
    auto deviceMntpts = _deviceInfo->mountpoints();
    auto deviceVolumesNames = QList<QString>();

    for (auto const& mntpt : deviceMntpts) {
        deviceVolumesNames.push_back(mntpt.filePath().replace("\\\\.\\",""));
    }

    auto mountedVolumes = QStorageInfo::mountedVolumes();
    auto bootVolume = QString();

    for (auto const& volume : mountedVolumes) {
        for (auto const& devVolName : deviceVolumesNames) {
            if (volume.rootPath().startsWith(devVolName) && volume.name() == "boot") {
                bootVolume = devVolName;
            }
        }
    }

    if (bootVolume.isEmpty()) {
        return fwVersion;
    }

    auto issuePath = bootVolume + _fwVersionFilename();
    qInfo() << "Search version in " << issuePath;
#else
    /// get boot partition
    auto optBootPartition = _bootPartition();
    if (!optBootPartition.present()) {
        return fwVersion;
    }

    auto bootPartition = optBootPartition.release();

    /// get mountpoint of boot part
    auto optBootPartMntpt = _bootPartMountpoint(bootPartition);
    if (!optBootPartMntpt.present()) {
       return fwVersion;
    }

    auto bootPartMntpt = optBootPartMntpt.release();

    /// get version
    auto issuePath = bootPartMntpt.filePath() + _fwVersionFilename();
#endif
    QFile issueFile(issuePath);

    auto succeed = issueFile.open(QIODevice::ReadOnly);

    if (!succeed) {
        qWarning() << "Failed to open: " << issuePath;
        return fwVersion;
    }

    auto regexp = QRegExp("^v\\d\\.\\d");

    while (!issueFile.atEnd()) {
        auto line = issueFile.readLine();
        auto pos = 0;

        if (regexp.indexIn(line, pos) != -1) {
             fwVersion = regexp.capturedTexts().at(0);
             qInfo() << "Edge version: " << fwVersion;
             break;
        }
    }

    issueFile.close();
#ifndef Q_OS_WIN
    bootPartMntpt.umount();
#endif

    return fwVersion;
}


util::Optional<devlib::Partition> edge::Device::_bootPartition(void)
{
    Q_ASSERT(_deviceInfo);
    auto partsFilter = [] (auto const& part) {
        return part.label() == "boot";
    };

    auto edgeParts = _deviceInfo->partitions();
    auto bootPart = std::find_if(edgeParts.begin(), edgeParts.end(), partsFilter);

    if (bootPart != edgeParts.end()) {
        return *bootPart;
    }

    return {};
}


util::Optional<devlib::Mountpoint> edge::Device::
    _bootPartMountpoint(devlib::Partition& bootPart)
{
    Q_ASSERT(_deviceInfo);
    auto bootPartMntpts = bootPart.mountpoints();

    if (!bootPartMntpts.empty()) {
        return bootPartMntpts.first();
    }

    auto dirManager = QDir();
    auto mntptName = _bootMountpointPath();

    if (!dirManager.mkpath(mntptName)){
        qWarning() << "can not make dir " << mntptName;
    }

    auto mntpt = bootPart.mount(mntptName);
    if (!mntpt.isValid()) {
        qCritical() << "can not mount " << mntptName;
        return {};
    }

    return mntpt;
}


QString edge::Device::_bootMountpointPath(void)
{
    return "/tmp/edge_boot_tmp";
}


QString edge::Device::_fwVersionFilename(void)
{
    return "/issue.txt";
}
