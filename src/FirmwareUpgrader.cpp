#include "FirmwareUpgrader.h"

#include "devapi/rpiboot/rpiboot.h"
#include "devapi/StorageDeviceManager.h"

#include "FirmwareUpgraderWatcher.h"
#include "ChecksumCalculator.h"
#include "Flasher.h"


FirmwareUpgrader::FirmwareUpgrader(QObject *parent)
    : QObject(parent)
{  }


void FirmwareUpgrader::setVidPid(int vid, QList<int> const& pids)
{
    _vid = vid;
    _pids = pids;
}


bool FirmwareUpgrader::runRpiBootStep(void)
{
    RpiBoot rpiboot(_vid, _pids);

    emit rpibootStateChanged(states::RpiBootState::RpiBootStarted,
                             states::StateType::Info);

    auto successful = rpiboot.rpiDevicesCount() > 0
            && rpiboot.bootAsMassStorage() == 0;

    if (!successful) {
        emit rpibootStateChanged(states::RpiBootState::RpiBootFailed,
                                 states::StateType::Error);
        qCritical() << "Rpiboot failed";
        return false;
    }

    emit rpibootStateChanged(states::RpiBootState::RpiBootFinished,
                             states::StateType::Info);

    return true;
}


bool FirmwareUpgrader::
    runDeviceScannerStep(void)
{
    emit deviceScannerStateChanged(states::DeviceScannerState::ScannerStarted);

    qInfo() << "Device detecting...";

    auto const sleepTime = 600;
    auto const maxPollingTime = 5000;

    auto manager = StorageDeviceManager::instance();
    auto totalSleepTime = 0;

    do {
        QThread::msleep(sleepTime);
        auto drives = manager->physicalDrives(_vid, _pids.at(0));

        if (drives.size() > 0) {
            _physicalDrives = std::move(drives);
            break;
        }

        totalSleepTime += sleepTime;

    } while (totalSleepTime < maxPollingTime);

    if (totalSleepTime > maxPollingTime) {
        emit deviceScannerStateChanged(states::DeviceScannerState::ScannerDeviceNotFound,
                                       states::StateType::Error);
        qCritical() << "Rpi as mass storage device not found";
        return false;
    }

    qInfo() << "Rpi device's found.";

    emit deviceMountpoints(QStringList(_physicalDrives.at(0)->mountpoints().toList()));

    emit deviceScannerStateChanged(states::DeviceScannerState::ScannerDeviceFound);
    emit deviceScannerStateChanged(states::DeviceScannerState::ScannerFinished);

    return true;
}


bool FirmwareUpgrader::
    runFlashingDeviceStep( QString const& firmwareFilename, bool checksumEnabled)
{
    for (auto const& device : _physicalDrives) {
        qInfo() << "About Device\n" << device-> toString();

        // First: unmount all mountpoints (it's required for windows and optional for linux)
        if (device->unmountAllMountpoints().failed()) {
            qWarning() << "Unmounting related mountpoints failed.";
        }

        // Try to open file with Image
        QFile imageFile(firmwareFilename);
        auto successful = imageFile.open(QIODevice::ReadOnly);

        if (!successful) {
            emit flasherStateChanged(states::FlasherState::FlasherOpenImageFailed,
                                     states::StateType::Error);
            qCritical() << "Failed to open image.";
            return false;
        }

        // Try to open file, which represent rpi device in filesystem
        QFile rpiDeviceFile;
        auto status = device->openAsQFile(&rpiDeviceFile);

        if (status.failed()) {
            emit flasherStateChanged(states::FlasherState::FlasherDeviceWritingFailed,
                                     states::StateType::Error);
            qCritical() << "Failed to open as qfile.";
            return false;
        }

        // Connect all signals for receiving flasher state
        Flasher flasher;

        connect(&flasher, &Flasher::flashStarted,    this, &FirmwareUpgrader::_onFlashStarted);
        connect(&flasher, &Flasher::flashFailed,     this, &FirmwareUpgrader::_onFlashFailed);
        connect(&flasher, &Flasher::progressChanged, this, &FirmwareUpgrader::_onProgressChanged);

        // Lets flash our rpi device
        auto ioBlockSize = 1 << 16; // 64kb
        successful = flasher.flash(imageFile, rpiDeviceFile, ioBlockSize);

        if (!successful) {
            emit flasherStateChanged(states::FlasherState::FlasherFailed,
                                     states::StateType::Error);
            qCritical() << "Flashing failed";
            return false;
        }

        if (checksumEnabled) {
            successful = _checkCorrectness(imageFile, rpiDeviceFile);
            if (!successful) {
                qWarning("image uncorrectly wrote.");
                emit flasherStateChanged(states::FlasherState::FlasherImageUncorrectlyWrote,
                                         states::StateType::Warning);
            }
            emit flasherStateChanged(states::FlasherState::FlasherImageCorrectlyWrote);
        }

        emit flasherStateChanged(states::FlasherState::FlasherFinished);
    }

    return true;
}


bool FirmwareUpgrader::_checkCorrectness(QFile& image, QFile& device)
{
    ChecksumCalculator calc;

    connect(&calc, &ChecksumCalculator::progressChanged, this, &FirmwareUpgrader::flashingProgressChanged);

    auto imgChecksum = calc.calculate(image, image.size());
    auto devChecksum = calc.calculate(device, image.size());

    return imgChecksum == devChecksum;
}


void FirmwareUpgrader::_onFlashStarted()
{
    qInfo() << "Flashing started.";
    emit flasherStateChanged(states::FlasherState::FlasherStarted);
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
        qCritical() << "Flasher: image reading failed";
        emit flasherStateChanged(states::FlasherState::FlasherImageReadingFailed,
                                 states::StateType::Error);
    } else {
        qCritical() << "Flasher: device writing failed";
        emit flasherStateChanged(states::FlasherState::FlasherDeviceWritingFailed,
                                 states::StateType::Error);
    }
}
