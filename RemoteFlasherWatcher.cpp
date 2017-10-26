#include "rpiboot/rpiboot.h"

#include "RemoteFlasherWatcher.h"
#include "StorageDeviceManager.h"
#include "Flasher.h"


RemoteFlasherWatcher::RemoteFlasherWatcher(QObject *parent)
    : FlasherWatcherSimpleSource(parent)
{ }


void RemoteFlasherWatcher::start(void)
{
    auto const requiredVid = 0x0a5c;
    auto requiredPids = QList<int>({ 0x2764, 0x2763 });

    RpiBoot rpiboot(requiredVid, requiredPids);

    if (rpiboot.rpiDevicesCount() == 0) {
        setRpiBootState(FlasherState::DeviceNotFound);
        qCritical() << "Not rpi devices found.";
        return;
    }

    setRpiBootState(FlasherState::Started);
    auto successful = rpiboot.bootAsMassStorage();

    if (!successful) {
        setRpiBootState(FlasherState::UnexpectedError);
        qCritical() << "Rpiboot failed";
        return;
    }

    setRpiBootState(FlasherState::Finished);
    setDeviceScannerState(FlasherState::Started);

    auto manager = StorageDeviceManager::instance();
    auto physicalDrives = manager->physicalDrives(requiredVid, requiredPids.at(0));

    qInfo() << "Devices count: " << physicalDrives.size();

    if (physicalDrives.size() == 0) {
        setDeviceScannerState(FlasherState::DeviceNotFound);
        qCritical() << "Rpi ass mass storage device not found";
        return;
    }

    setDeviceScannerState(FlasherState::DeviceFound);
    setFirmwareUpgraderState(FlasherState::Started);

    for (auto device : physicalDrives) {
        qInfo() << "About Device\n" << device-> toString();

        // First: unmount all mountpoints (it's required for windows and optional for linux)
        device->unmountAllMountpoints();

        // Try to open file with Image
        QFile imageFile("/home/vladimir.provalov/Downloads/emlid-raspbian-20170323.img");
        successful = imageFile.open(QIODevice::ReadOnly);

        if (!successful) {
            setFirmwareUpgraderState(FlasherState::OpenImageFailed);
            qCritical() << "Failed to open image.";
            return;
        }

        // Try to open file, which represent rpi device in filesystem
        QFile rpiDeviceFile;
        auto status = device->openAsQFile(&rpiDeviceFile);

        if (status.failed()) {
            setFirmwareUpgraderState(FlasherState::OpenDeviceFailed);
            qCritical() << "Failed to open as qfile.";
            return;
        }


        // Connect all for receiving flasher state
        Flasher flasher;

        connect(&flasher, &Flasher::flashStarted,    this, &RemoteFlasherWatcher::onFlashStarted);
        connect(&flasher, &Flasher::flashCompleted,  this, &RemoteFlasherWatcher::onFlashCompleted);
        connect(&flasher, &Flasher::flashFailed,     this, &RemoteFlasherWatcher::onFlashFailed);
        connect(&flasher, &Flasher::progressChanged, this, &RemoteFlasherWatcher::onProgressChanged);

        // Lets flashing our rpi device
        successful = flasher.flash(imageFile, rpiDeviceFile);

        if (!successful) {
            qCritical() << "Flashing failed";
            return;
        }
    }

}


void RemoteFlasherWatcher::
    setRpiBootState(RemoteFlasherWatcher::FlasherState value)
{
    emit rpiBootStateChanged(value);
}


void RemoteFlasherWatcher::
    setDeviceScannerState(RemoteFlasherWatcher::FlasherState value)
{
    emit deviceScannerStateChanged(value);
}


void RemoteFlasherWatcher::
    setFirmwareUpgraderState(RemoteFlasherWatcher::FlasherState value)
{
    emit firmwareUpgraderStateChanged(value);
}


void RemoteFlasherWatcher::onFlashStarted()
{
    setFirmwareUpgraderState(FlasherState::Started);
}


void RemoteFlasherWatcher::onFlashCompleted()
{
    setFirmwareUpgraderState(FlasherState::Finished);
}


void RemoteFlasherWatcher::onProgressChanged(uint progress)
{
    emit flashingProgressChanged(progress);
}


void RemoteFlasherWatcher::
    onFlashFailed(Flasher::FlashingStatus status)
{
    if (status == Flasher::FlashingStatus::READ_FAILED) {
       setFirmwareUpgraderState(FlasherState::ImageReadingFailed);
    } else {
       setFirmwareUpgraderState(FlasherState::DeviceWritingFailed);
    }
}
