#include "FirmwareUpgrader.h"

#include "rpiboot/rpiboot.h"

#include "FirmwareUpgraderWatcher.h"
#include "StorageDeviceManager.h"
#include "Flasher.h"


FirmwareUpgrader::FirmwareUpgrader(QObject *parent)
    : QObject(parent)
{  }


void FirmwareUpgrader::
    _setCurrentState(QString const& subsystem, FirmwareUpgrader::State state)
{
    Q_EMIT subsystemStateChanged(subsystem, state);
}


bool FirmwareUpgrader::runRpiBootStep(int vid, QList<int> const& pids)
{
    auto setRpiBootState = [this] (FirmwareUpgrader::State const& state)
        { _setCurrentState("RpiBoot", state); };

    RpiBoot rpiboot(vid, pids);

    setRpiBootState(FirmwareUpgrader::State::Started);

    auto successful = rpiboot.rpiDevicesCount() > 0
            && rpiboot.bootAsMassStorage() == 0;

    if (!successful) {
        setRpiBootState(FirmwareUpgrader::State::DeviceNotFound);
        qCritical() << "Rpiboot failed";
        return false;
    }

    setRpiBootState(FirmwareUpgrader::State::Finished);
    return true;
}


bool FirmwareUpgrader::
    runDeviceScannerStep(int vid, QList<int> const& pids, QVector<std::shared_ptr<StorageDevice>>* physicalDrives)
{
    auto setDeviceScannerState = [this] (FirmwareUpgrader::State const& state)
        { _setCurrentState("DeviceScanner", state); };

    setDeviceScannerState(FirmwareUpgrader::State::Started);

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
        setDeviceScannerState(FirmwareUpgrader::State::DeviceNotFound);
        qCritical() << "Rpi as mass storage device not found";
        return false;
    }

    qInfo() << "Rpi device's found.";
    setDeviceScannerState(FirmwareUpgrader::State::DeviceFound);

    return true;
}


bool FirmwareUpgrader::
    runFlashingDeviceStep(QVector<std::shared_ptr<StorageDevice>> const& physicalDrives)
{
    auto setFlasherState = [this] (FirmwareUpgrader::State const& state)
        { _setCurrentState("Flasher", state); };

    for (auto device : physicalDrives) {
        qInfo() << "About Device\n" << device-> toString();

        // First: unmount all mountpoints (it's required for windows and optional for linux)
        device->unmountAllMountpoints();

        // Try to open file with Image
        QFile imageFile("/home/vladimir.provalov/Downloads/emlid-raspbian-20170323.img");
        auto successful = imageFile.open(QIODevice::ReadOnly);

        if (!successful) {
            setFlasherState(FirmwareUpgrader::State::OpenImageFailed);
            qCritical() << "Failed to open image.";
            return false;
        }

        // Try to open file, which represent rpi device in filesystem
        QFile rpiDeviceFile;
        auto status = device->openAsQFile(&rpiDeviceFile);

        if (status.failed()) {
            setFlasherState(FirmwareUpgrader::State::OpenDeviceFailed);
            qCritical() << "Failed to open as qfile.";
            return false;
        }

        // Connect all for receiving flasher state
        Flasher flasher;

        connect(&flasher, &Flasher::flashStarted,    this, &FirmwareUpgrader::_onFlashStarted);
        connect(&flasher, &Flasher::flashCompleted,  this, &FirmwareUpgrader::_onFlashCompleted);
        connect(&flasher, &Flasher::flashFailed,     this, &FirmwareUpgrader::_onFlashFailed);
        connect(&flasher, &Flasher::progressChanged, this, &FirmwareUpgrader::_onProgressChanged);

        // Lets flashing our rpi device
        successful = flasher.flash(imageFile, rpiDeviceFile);

        if (!successful) {
            qCritical() << "Flashing failed";
            return false;
        }
    }

    return false;
}


void FirmwareUpgrader::start(void)
{
    qInfo() << ":: Firmware upgrader started ::";
    _runAllSteps();
    qInfo() << ":: Firmware upgrader finished ::";
}


void FirmwareUpgrader::_runAllSteps(void)
{
    auto const requiredVid = 0x0a5c;
    auto requiredPids = QList<int>({ 0x2764, 0x2763 });

    QVector<std::shared_ptr<StorageDevice>> physicalDrives;

    runRpiBootStep(requiredVid, requiredPids)
            && runDeviceScannerStep(requiredVid, requiredPids, &physicalDrives)
            && runFlashingDeviceStep(physicalDrives);
}


void FirmwareUpgrader::_onFlashStarted()
{
    qInfo() << "Flashing started.";
    _setCurrentState("Flasher", FirmwareUpgrader::State::Started);
}


void FirmwareUpgrader::_onFlashCompleted()
{
    qInfo() << "Flashing finished.";
    _setCurrentState("Flasher", FirmwareUpgrader::State::Finished);
}


void FirmwareUpgrader::_onProgressChanged(uint progress)
{
    qInfo() << "Progress: " << progress;
    emit flashingProgressChanged(progress);
}


void FirmwareUpgrader::
    _onFlashFailed(Flasher::FlashingStatus status)
{
    if (status == Flasher::FlashingStatus::READ_FAILED) {
       _setCurrentState("Flasher", FirmwareUpgrader::State::ImageReadingFailed);
    } else {
       _setCurrentState("Flasher", FirmwareUpgrader::State::DeviceWritingFailed);
    }
}
