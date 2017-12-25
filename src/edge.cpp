#include "edge.h"
#include "usb/usb.h"

/// functions

bool edge::available(void)
{
    return usb::countOfDevicesWith(edge::vid(), {edge::pid()}) > 0;
}


int edge::vid(void) {
    return 0x0a5c;
}

int edge::pid(void) {
    return 0x2764;
}

int edge::pidAsMassStorage(void) {
    return 0x0001;
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
        return util::make_optional(function);
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

    auto const maxPollingTime = 10000;
    auto const sleepTime      = 600;
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
            return util::make_optional(edge::Device(*edgePtr));
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


edge::Device::Device(Device&& device)
    : _deviceInfo(std::move(device._deviceInfo))
{ }


edge::Device::Device(Device const& device)
{
    if (device._deviceInfo != nullptr) {
        auto const& ptr = *(device._deviceInfo);
        _deviceInfo.reset(new devlib::StorageDeviceInfo(ptr));
    } else {
        _deviceInfo.reset(nullptr);
    }
}


edge::Device edge::Device::operator =(Device&& device)
{
    return Device(std::move(device));
}


edge::Device edge::Device::operator =(Device const& device)
{
    return Device(device);
}


bool edge::Device::stillAvailable(void) const
{
    auto devices = devlib::StorageDeviceInfo::availableDevices();
    return std::any_of(devices.cbegin(), devices.cend(), edge::isEdgeAsMassStorage);
}


QString edge::Device::firmwareVersion(void)
{

    auto fwVersion = QString();

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
             break;
        }
    }

    bootPartMntpt.umount();

    return fwVersion;
}


util::Optional<devlib::Partition> edge::Device::_bootPartition(void)
{
    auto partsFilter = [] (auto const& part) {
        return part.label() == "boot";
    };

    auto edgeParts = _deviceInfo->partitions();
    auto bootPart = std::find_if(edgeParts.begin(), edgeParts.end(), partsFilter);

    if (bootPart != edgeParts.end()) {
        return util::make_optional(*bootPart);
    }

    return {};
}


util::Optional<devlib::Mountpoint> edge::Device::
    _bootPartMountpoint(devlib::Partition& bootPart)
{
    auto bootPartMntpts = bootPart.mountpoints();

    if (!bootPartMntpts.empty()) {
        return util::make_optional(bootPartMntpts.first());
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
