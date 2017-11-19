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
    using Watcher = FirmwareUpgraderWatcher;
    using FwUpgrader = FirmwareUpgrader;

    QObject::connect(&_thread, &QThread::finished, fwUpgrader, &FwUpgrader::deleteLater);

    QObject::connect(fwUpgrader, &FwUpgrader::flashingProgressChanged,   this, &Watcher::flasherProgressChanged);
    QObject::connect(fwUpgrader, &FwUpgrader::deviceMountpoints,         this, &Watcher::deviceMountpoints);

    QObject::connect(fwUpgrader, &FwUpgrader::flasherStateChanged,       this, &Watcher::_onFlasherStateChanged);
    QObject::connect(fwUpgrader, &FwUpgrader::deviceScannerStateChanged, this, &Watcher::_onDeviceScannerStateChanged);
    QObject::connect(fwUpgrader, &FwUpgrader::rpibootStateChanged,       this, &Watcher::_onRpiBootStateChanged);

    QObject::connect(this, &Watcher::_execRpiBoot,           fwUpgrader, &FwUpgrader::runRpiBootStep);
    QObject::connect(this, &Watcher::_execDeviceScannerStep, fwUpgrader, &FwUpgrader::runDeviceScannerStep);
    QObject::connect(this, &Watcher::_execFlasher,           fwUpgrader, &FwUpgrader::runFlashingDeviceStep);
    QObject::connect(this, &Watcher::_setVidPid,             fwUpgrader, &FwUpgrader::setVidPid);
    QObject::connect(this, &Watcher::_stop,                  fwUpgrader, &FwUpgrader::stop);

}


void FirmwareUpgraderWatcher::
    _onRpiBootStateChanged(states::RpiBootState state, states::StateType type)
{
    emit rpiBootStateChanged(static_cast<uint>(state), static_cast<uint>(type));

    using RBState = states::RpiBootState;

    if (state == RBState::RpiBootFinished) {
        emit rpiBootFinished(true);
    } else if (state == RBState::RpiBootFailed) {
        emit rpiBootFinished(false);
    }
}


void FirmwareUpgraderWatcher::
    _onDeviceScannerStateChanged(states::DeviceScannerState state, states::StateType type)
{
    emit deviceScannerStateChanged(static_cast<uint>(state), static_cast<uint>(type));

    using DevState = states::DeviceScannerState;

    if (state == DevState::ScannerFinished) {
        emit deviceScannerFinished(true);
    } else if (state == DevState::ScannerFailed) {
        emit deviceScannerFinished(false);
    }
}


void FirmwareUpgraderWatcher::
    _onFlasherStateChanged(states::FlasherState state, states::StateType type)
{
    emit flasherStateChanged(static_cast<uint>(state), static_cast<uint>(type));

    using FlashState = states::FlasherState;

    if (state == FlashState::FlasherFinished) {
        emit flasherFinished(true);
    } else if (state == FlashState::FlasherFailed) {
        emit flasherFinished(false);
    }
}


void FirmwareUpgraderWatcher::setVidPid(int vid, QList<int> pids)
{
    emit _setVidPid(vid, pids);
}


void FirmwareUpgraderWatcher::finish(void)
{
    emit _stop();
    if (!_thread.wait(10000)) {
        qWarning() << "thread didn't stop";
        _thread.terminate();
    }

    emit finished();
    qInfo() << "firmware upgrader finished";
    QCoreApplication::quit();
}


void FirmwareUpgraderWatcher::runRpiBootStep(void)
{
    emit _execRpiBoot();
}


void FirmwareUpgraderWatcher::runDeviceScannerStep(void)
{
    emit _execDeviceScannerStep();
}


void FirmwareUpgraderWatcher::runFlasherStep(QString firmwareFilename, bool checksumEnabled)
{
    qInfo() << "Firmware image filename: " << firmwareFilename;
    emit _execFlasher(firmwareFilename, checksumEnabled);
}
