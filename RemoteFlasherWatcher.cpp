#include <QSignalSpy>

#include "rpiboot/rpiboot.h"

#include "RemoteFlasherWatcher.h"
#include "StorageDeviceManager.h"
#include "Flasher.h"


RemoteFlasherWatcher::RemoteFlasherWatcher(QObject *parent)
    : FlasherWatcherSource(parent)
{ }


void RemoteFlasherWatcher::start(void)
{
    qInfo() << "Firmware upgrader started.";
    runAllSteps();
    qInfo() << "Firmware upgrader finished.";
}


void RemoteFlasherWatcher::
    setRpiBootState(RemoteFlasherWatcher::FlasherState value)
{
    Q_EMIT rpiBootStateChanged(value);
}


void RemoteFlasherWatcher::
    setDeviceScannerState(RemoteFlasherWatcher::FlasherState value)
{
    Q_EMIT deviceScannerStateChanged(value);
}


void RemoteFlasherWatcher::
    setFirmwareUpgraderState(RemoteFlasherWatcher::FlasherState value)
{
    Q_EMIT firmwareUpgraderStateChanged(value);
}


bool RemoteFlasherWatcher::runRpiBootStep(int vid, QList<int> const& pids)
{
    RpiBoot rpiboot(vid, pids);

    setRpiBootState(FlasherState::Started);

    if (rpiboot.rpiDevicesCount() == 0) {
        setRpiBootState(FlasherState::DeviceNotFound);
        qCritical() << "No one rpi devices found.";
        return false;
    }

    auto successful = rpiboot.bootAsMassStorage();

    if (successful != 0) {
        setRpiBootState(FlasherState::UnexpectedError);
        qCritical() << "Rpiboot failed";
        return false;
    }

    setRpiBootState(FlasherState::Finished);
    return true;
}


bool RemoteFlasherWatcher::
    runDeviceScannerStep(int vid, QList<int> const& pids, QVector<std::shared_ptr<StorageDevice>>* physicalDrives)
{
    setDeviceScannerState(FlasherState::Started);

    qInfo() << "Device detecting...";

    auto const sleepTime = 600;
    auto const maxPollingTime = 5000;

    auto manager = StorageDeviceManager::instance();
    auto totalSleepTime = 0;

    do {
        QThread::msleep(sleepTime);
        auto drives = manager->physicalDrives(vid, pids.at(0));

        if (drives.size() > 0) {
            (*physicalDrives) = std::move(drives);
            break;
        }

        totalSleepTime += sleepTime;

    } while (totalSleepTime < maxPollingTime);

    if (totalSleepTime > maxPollingTime) {
        setDeviceScannerState(FlasherState::DeviceNotFound);
        qCritical() << "Rpi as mass storage device not found";
        return false;
    }

    qInfo() << "Rpi device's found.";
    setDeviceScannerState(FlasherState::DeviceFound);

    return true;
}


bool RemoteFlasherWatcher::
    runFlashingDeviceStep(QVector<std::shared_ptr<StorageDevice>> const& physicalDrives)
{
    setFirmwareUpgraderState(FlasherState::Started);

    for (auto device : physicalDrives) {
        qInfo() << "About Device\n" << device-> toString();

        // First: unmount all mountpoints (it's required for windows and optional for linux)
        device->unmountAllMountpoints();

        // Try to open file with Image
        QFile imageFile("/home/vladimir.provalov/Downloads/emlid-raspbian-20170323.img");
        auto successful = imageFile.open(QIODevice::ReadOnly);

        if (!successful) {
            setFirmwareUpgraderState(FlasherState::OpenImageFailed);
            qCritical() << "Failed to open image.";
            return false;
        }

        // Try to open file, which represent rpi device in filesystem
        QFile rpiDeviceFile;
        auto status = device->openAsQFile(&rpiDeviceFile);

        if (status.failed()) {
            setFirmwareUpgraderState(FlasherState::OpenDeviceFailed);
            qCritical() << "Failed to open as qfile.";
            return false;
        }

        // Connect all for receiving flasher state
        Flasher flasher;

        connect(&flasher, &Flasher::flashStarted,    this, &RemoteFlasherWatcher::_onFlashStarted);
        connect(&flasher, &Flasher::flashCompleted,  this, &RemoteFlasherWatcher::_onFlashCompleted);
        connect(&flasher, &Flasher::flashFailed,     this, &RemoteFlasherWatcher::_onFlashFailed);
        connect(&flasher, &Flasher::progressChanged, this, &RemoteFlasherWatcher::_onProgressChanged);

        // Lets flashing our rpi device
        successful = flasher.flash(imageFile, rpiDeviceFile);

        if (!successful) {
            qCritical() << "Flashing failed";
            return false;
        }
    }

    return false;
}


void RemoteFlasherWatcher::runAllSteps(void)
{
    auto const requiredVid = 0x0a5c;
    auto requiredPids = QList<int>({ 0x2764, 0x2763 });

    QVector<std::shared_ptr<StorageDevice>> physicalDrives;

    runRpiBootStep(requiredVid, requiredPids)
            && runDeviceScannerStep(requiredVid, requiredPids, &physicalDrives)
            && runFlashingDeviceStep(physicalDrives);
}


void RemoteFlasherWatcher::_onFlashStarted()
{
    qInfo() << "Flashing started.";
    setFirmwareUpgraderState(FlasherState::Started);
}


void RemoteFlasherWatcher::_onFlashCompleted()
{
    qInfo() << "Flashing finished.";
    setFirmwareUpgraderState(FlasherState::Finished);
}


void RemoteFlasherWatcher::_onProgressChanged(uint progress)
{
    qInfo() << "Progress: " << progress;
    emit flashingProgressChanged(progress);
}


void RemoteFlasherWatcher::
    _onFlashFailed(Flasher::FlashingStatus status)
{
    if (status == Flasher::FlashingStatus::READ_FAILED) {
       setFirmwareUpgraderState(FlasherState::ImageReadingFailed);
    } else {
       setFirmwareUpgraderState(FlasherState::DeviceWritingFailed);
    }
}
