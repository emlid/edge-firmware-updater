#include <memory>

#include "FirmwareUpgraderWatcher.h"
#include "FirmwareUpgrader.h"
#include "shared/States.h"

FirmwareUpgraderWatcher::FirmwareUpgraderWatcher(QObject *parent)
    : FirmwareUpgraderWatcherSimpleSource(parent)
{
    auto fwUpgrader = new FirmwareUpgrader();
    fwUpgrader->moveToThread(&_thread);
    _initConnections(fwUpgrader);
    _thread.start();
}


void FirmwareUpgraderWatcher::_initConnections(FirmwareUpgrader* fwUpgrader)
{
    QObject::connect(&_thread, &QThread::finished, fwUpgrader, &FirmwareUpgrader::deleteLater);
    QObject::connect(fwUpgrader, &FirmwareUpgrader::flashingProgressChanged,
                     this, &FirmwareUpgraderWatcher::flasherProgressChanged);

    QObject::connect(fwUpgrader, &FirmwareUpgrader::deviceMountpoints,
                     this, &FirmwareUpgraderWatcher::deviceMountpoints);

    // Signals with states
    QObject::connect(fwUpgrader, &FirmwareUpgrader::flasherStateChanged,
                     this, &FirmwareUpgraderWatcher::_onFlasherStateChanged);

    QObject::connect(fwUpgrader, &FirmwareUpgrader::deviceScannerStateChanged,
                     this, &FirmwareUpgraderWatcher::_onDeviceScannerStateChanged);

    QObject::connect(fwUpgrader, &FirmwareUpgrader::rpibootStateChanged,
                     this, &FirmwareUpgraderWatcher::_onRpiBootStateChanged);

    QObject::connect(this, &FirmwareUpgraderWatcher::execRpiBoot,
                     fwUpgrader, &FirmwareUpgrader::runRpiBootStep);

    QObject::connect(this, &FirmwareUpgraderWatcher::execDeviceScannerStep,
                     fwUpgrader, &FirmwareUpgrader::runDeviceScannerStep);

    QObject::connect(this, &FirmwareUpgraderWatcher::execFlasher,
                     fwUpgrader, &FirmwareUpgrader::runFlashingDeviceStep);

    QObject::connect(this, &FirmwareUpgraderWatcher::setVidPid,
                     fwUpgrader, &FirmwareUpgrader::setVidPid);

}


void FirmwareUpgraderWatcher::
    _onRpiBootStateChanged(states::RpiBootState state, states::StateType type)
{
    emit rpiBootStateChanged(static_cast<uint>(state), static_cast<uint>(type));

    if (state == states::RpiBootState::RpiBootFinished) {
        emit rpiBootFinished(true);
    } else if (state == states::RpiBootState::RpiBootFailed) {
        emit rpiBootFinished(false);
    }
}


void FirmwareUpgraderWatcher::
    _onDeviceScannerStateChanged(states::DeviceScannerState state, states::StateType type)
{
    emit deviceScannerStateChanged(static_cast<uint>(state), static_cast<uint>(type));

    if (state == states::DeviceScannerState::ScannerFinished) {
        emit deviceScannerFinished(true);
    } else if (state == states::DeviceScannerState::ScannerFailed) {
        emit deviceScannerFinished(false);
    }
}


void FirmwareUpgraderWatcher::
    _onFlasherStateChanged(states::FlasherState state, states::StateType type)
{
    emit flasherStateChanged(static_cast<uint>(state), static_cast<uint>(type));

    if (state == states::FlasherState::FlasherFinished) {
        emit flasherFinished(true);
    } else if (state == states::FlasherState::FlasherFailed) {
        emit flasherFinished(false);
    }
}


void FirmwareUpgraderWatcher::setFilterParams(int vid, QList<int> pids)
{
    emit setVidPid(vid, pids);
}


void FirmwareUpgraderWatcher::cancel(void)
{
    _thread.quit();
    _thread.wait();

    emit cancelled();
    _thread.start();
}

void FirmwareUpgraderWatcher::finish(void)
{
    qInfo() << "firmware upgrader finished";
    cancel();
    emit finished();

    std::exit(EXIT_SUCCESS);
}


void FirmwareUpgraderWatcher::runRpiBootStep(void)
{
    emit execRpiBoot();
}


void FirmwareUpgraderWatcher::runDeviceScannerStep(void)
{
    emit execDeviceScannerStep();
}


void FirmwareUpgraderWatcher::runFlasherStep(QString firmwareFilename, bool checksumEnabled)
{
    qInfo() << "Firmware image filename: " << firmwareFilename;

    emit execFlasher(firmwareFilename, checksumEnabled);
}
