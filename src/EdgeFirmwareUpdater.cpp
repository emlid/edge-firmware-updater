#include <memory>

#include "EdgeFirmwareUpdater.h"
#include "DeviceInitializerSubtask.h"
#include "FlasherSubtask.h"
#include "ChecksumSubtask.h"


EdgeFirmwareUpdater::EdgeFirmwareUpdater(QObject *parent)
    : EdgeFirmwareUpdaterIPCSimpleSource(parent)
{
    QObject::connect(this,          &EdgeFirmwareUpdater::_cancel,
                     &_taskManager, &SubtaskManager::stopTask);
}


void EdgeFirmwareUpdater::initializeEdgeDevice(void)
{
    using Upgrader    = EdgeFirmwareUpdater;
    using Initializer = DeviceInitializerSubtask;

    auto initSubtask = std::unique_ptr<Initializer>(new Initializer());
    QObject::connect(initSubtask.get(), &Initializer::logMessage,
                     this,              &Upgrader::_onLogMessageReceived);
    QObject::connect(initSubtask.get(), &Initializer::finished,
                     this,              &Upgrader::initializingFinished);
    QObject::connect(initSubtask.get(), &Initializer::deviceAvailable,
                    [this] (auto device) { _edgeDevice = util::make_optional(device); });

    _taskManager.run(std::move(initSubtask));
}


void EdgeFirmwareUpdater::flash(QString firmwareFilename)
{
    using FlasherTask = FlasherSubtask;
    using Upgrader    = EdgeFirmwareUpdater;

    while (_taskManager.hasActiveSubtasks()) {
        QCoreApplication::processEvents();
    }

    if (!_edgeDevice.present()) {
        qFatal("edge device not present, but flasher executed.");
        return;
    }

    auto edgeAsStorage = _edgeDevice.get().asStorageDevice();

    // First: unmount all mountpoints (it's required for windows and optional for linux)
    auto edgeMntpts = edgeAsStorage.mountpoints();
    for (auto& mntpt : edgeMntpts) {
        mntpt.umount();
    }

    auto succeed = _flasherData.reset(firmwareFilename, edgeAsStorage);
    if (!succeed) {
        qFatal("Previous flasher data not released.");
        return;
    }

    // Try to open file with Image
    succeed = _flasherData.image()
            ->open(QIODevice::ReadOnly);

    if (!succeed) {
        emit logMessage("can not open image file", AbstractSubtask::Error);
        return;
    }

    // Try to open file, which represent rpi device in filesystem
    succeed = _flasherData.device()
            ->open(QIODevice::ReadWrite | QIODevice::Unbuffered);

    if (!succeed) {
        emit logMessage("can not open device file", AbstractSubtask::Error);
        if (_edgeDevice.get().stillAvailable()) {
             emit logMessage("edge device currently unavailable", AbstractSubtask::Error);
        }
        qCritical() << "Failed to open as qfile.";
        return;
    }

    auto flasherSubtask = std::unique_ptr<FlasherTask>
            (new FlasherTask(_flasherData.image(), _flasherData.device()));
    QObject::connect(flasherSubtask.get(), &FlasherTask::logMessage,
                     this,                 &Upgrader::_onLogMessageReceived);
    QObject::connect(flasherSubtask.get(), &FlasherTask::finished,
                     this,                 &Upgrader::flashingFinished);
    QObject::connect(flasherSubtask.get(), &FlasherTask::progressChanged,
                     this,                 &Upgrader::progressChanged);

    _taskManager.run(std::move(flasherSubtask));
}


void EdgeFirmwareUpdater::checkOnCorrectness(void)
{
    using CsSubtask = ChecksumSubtask;
    using Upgrader   = EdgeFirmwareUpdater;

    while (_taskManager.hasActiveSubtasks()) {
        QCoreApplication::processEvents();
    }

    auto csSubtask = std::unique_ptr<CsSubtask>
            (new CsSubtask(_flasherData.image(), _flasherData.device()));

    QObject::connect(csSubtask.get(), &CsSubtask::logMessage,
                     this,            &Upgrader::_onLogMessageReceived);
    QObject::connect(csSubtask.get(), &CsSubtask::progressChanged,
                     this,            &Upgrader::progressChanged);
    QObject::connect(csSubtask.get(), &CsSubtask::finished,
                     this,            &Upgrader::checkOnCorrectnessFinished);

    _taskManager.run(std::move(csSubtask));
}


void EdgeFirmwareUpdater::finish(void)
{
    if (_taskManager.hasActiveSubtasks()) {
        qInfo() << "The process has active tasks. Wait for finished.";
        QObject::connect(&_taskManager, &SubtaskManager::noActiveSubtasks,
                         this,          &EdgeFirmwareUpdater::_exit);
        emit _cancel();
    } else {
        qInfo() << "The process has no active tasks. Exit.";
        _exit();
    }
}


void EdgeFirmwareUpdater::cancel(void)
{
    if (_taskManager.hasActiveSubtasks()) {
        qInfo() << "The process has active tasks. Wait for finished.";
        QObject::connect(&_taskManager, &SubtaskManager::noActiveSubtasks,
                         this,          &EdgeFirmwareUpdater::_onNoActiveTasks);
        emit _cancel();
    } else {
        qInfo() << "The process has no active tasks. Cancellation.";
        emit cancelled();
    }
}


void EdgeFirmwareUpdater::_exit(void)
{
    std::exit(EXIT_SUCCESS);
}


void EdgeFirmwareUpdater::_onNoActiveTasks() {
    QObject::disconnect(&_taskManager, &SubtaskManager::noActiveSubtasks,
                        this,          &EdgeFirmwareUpdater::_onNoActiveTasks);
    emit cancelled();
}


void EdgeFirmwareUpdater::
    _onLogMessageReceived(QString msg, AbstractSubtask::LogMessageType type)
{
    emit logMessage(msg, static_cast<int>(type));
}


