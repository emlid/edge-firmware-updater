#include <memory>

#include "shared/States.h"
#include "FirmwareUpgraderWatcher.h"
#include "RpiBootSubtask.h"
#include "DeviceScannerSubtask.h"
#include "FlasherSubtask.h"
#include "ChecksumSubtask.h"


FirmwareUpgraderWatcher::FirmwareUpgraderWatcher(QObject *parent)
    : FirmwareUpgraderWatcherSimpleSource(parent),
      _image(nullptr),
      _device(nullptr)
{
    QObject::connect(this,          &FirmwareUpgraderWatcher::_stop,
                     &_taskManager, &SubtaskManager::stopTask);
}


void FirmwareUpgraderWatcher::
    _onRpiBootStateChanged(states::RpiBootState state, states::StateType type)
{
    emit rpiBootStateChanged(static_cast<uint>(state), static_cast<uint>(type));
}


void FirmwareUpgraderWatcher::
    _onDeviceScannerStateChanged(states::DeviceScannerState state, states::StateType type)
{
    emit deviceScannerStateChanged(static_cast<uint>(state), static_cast<uint>(type));
}


void FirmwareUpgraderWatcher::
    _onFlasherStateChanged(states::FlasherState state, states::StateType type)
{
    emit flasherStateChanged(static_cast<uint>(state), static_cast<uint>(type));
}

void FirmwareUpgraderWatcher::
    _onChecksumCalcStateChanged(states::CheckingCorrectnessState state, states::StateType type)
{
    emit checkingCorrectnessStateChanged(static_cast<uint>(state), static_cast<uint>(type));
}


void FirmwareUpgraderWatcher::setVidPid(int vid, QList<int> pids)
{
    _vid  = vid;
    _pids = pids;
}


void FirmwareUpgraderWatcher::finish(void)
{
    if (_taskManager.hasActiveSubtasks()) {
        qInfo() << "The process has active tasks. Wait for finished.";
        QObject::connect(&_taskManager, &SubtaskManager::noActiveSubtasks,
                         this,          &FirmwareUpgraderWatcher::_exit);
        emit _stop();
    } else {
        qInfo() << "The process has no active tasks. Exit.";
        _exit();
    }
}


void FirmwareUpgraderWatcher::_exit(void)
{
    std::exit(EXIT_SUCCESS);
}


void FirmwareUpgraderWatcher::runRpiBootStep(void)
{
    using RBoot   = RpiBootSubtask;
    using Watcher = FirmwareUpgraderWatcher;

    auto rbSubtask = new RBoot(_vid, _pids);
    QObject::connect(rbSubtask, &RBoot::stateChanged, this, &Watcher::_onRpiBootStateChanged);
    QObject::connect(rbSubtask, &RBoot::finished,     this, &Watcher::rpiBootFinished);

    _taskManager.run(rbSubtask);
}


void FirmwareUpgraderWatcher::runDeviceScannerStep(void)
{
    using DScanner = DeviceScannerSubtask;
    using Watcher  = FirmwareUpgraderWatcher;

    auto dsSubtask = new DScanner(_vid, _pids);
    QObject::connect(dsSubtask, &DScanner::stateChanged, this, &Watcher::_onDeviceScannerStateChanged);
    QObject::connect(dsSubtask, &DScanner::finished,     this, &Watcher::deviceScannerFinished);
    QObject::connect(dsSubtask, &DScanner::finished,
        [dsSubtask, this] () { _devices = dsSubtask->result(); });

    _taskManager.run(dsSubtask);
}


void FirmwareUpgraderWatcher::runCheckingCorrectnessStep(void)
{
    using CsSubtask = ChecksumSubtask;
    using Watcher   = FirmwareUpgraderWatcher;

    auto csSubtask = new CsSubtask(_image, _device);
    QObject::connect(csSubtask, &CsSubtask::stateChanged,
                     this,      &Watcher::_onChecksumCalcStateChanged);
    QObject::connect(csSubtask, &CsSubtask::progressChanged,
                     this,      &Watcher::checkingCorrectnessProgressChanged);
    QObject::connect(csSubtask, &CsSubtask::finished,
                     this,      &Watcher::checkingCorrectnessFinished);

    _taskManager.run(csSubtask);
}


void FirmwareUpgraderWatcher::runFlasherStep(QString firmwareFilename)
{
    using FlasherTask = FlasherSubtask;
    using Watcher     = FirmwareUpgraderWatcher;
    using FlState     = states::FlasherState;
    using StType      = states::StateType;

    auto device = _devices[0];

    _image.reset(new QFile(firmwareFilename));
    _device.reset(new QFile());

    qInfo() << "About Device\n" << device-> toString();

    // First: unmount all mountpoints (it's required for windows and optional for linux)
    if (device->unmountAllMountpoints().failed()) {
        qWarning() << "Unmounting related mountpoints failed.";
    }

    // Try to open file with Image
    auto successful = _image->open(QIODevice::ReadOnly);

    if (!successful) {
        emit flasherStateChanged(FlState::FlasherOpenImageFailed, StType::Error);
        qCritical() << "Failed to open image.";
        return;
    }

    // Try to open file, which represent rpi device in filesystem
    auto status = device->openAsQFile(_device.get());

    if (status.failed()) {
        emit flasherStateChanged(FlState::FlasherDeviceWritingFailed, StType::Error);
        qCritical() << "Failed to open as qfile.";
        return;
    }

    auto flasherSubtask = new FlasherTask(_image, _device);
    QObject::connect(flasherSubtask, &FlasherTask::stateChanged,    this, &Watcher::_onFlasherStateChanged);
    QObject::connect(flasherSubtask, &FlasherTask::finished,        this, &Watcher::flasherFinished);
    QObject::connect(flasherSubtask, &FlasherTask::progressChanged, this, &Watcher::flasherProgressChanged);

    _taskManager.run(flasherSubtask);
}
