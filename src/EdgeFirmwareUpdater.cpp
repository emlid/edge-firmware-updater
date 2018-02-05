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
    QObject::connect(initSubtask.get(), &Initializer::edgeVersion,
                     this,              &Upgrader::firmwareVersion);
    QObject::connect(initSubtask.get(), &Initializer::logMessage,
                     this,              &Upgrader::_onLogMessageReceived);
    QObject::connect(initSubtask.get(), &Initializer::finished,
                     this,              &Upgrader::initializingFinished);
    QObject::connect(initSubtask.get(), &Initializer::deviceAvailable,
        [this] (auto device) {
             qInfo() << "received";
             _edgeDevice.reset(device);
             qInfo() << _edgeDevice.present();
         }
    );

    _taskManager.run(std::move(initSubtask));
}


void EdgeFirmwareUpdater::flash(QString firmwareFilename)
{
    using FlasherTask = FlasherSubtask;
    using Upgrader    = EdgeFirmwareUpdater;

    while (_taskManager.hasActiveSubtasks()) {
        QCoreApplication::processEvents();
    }

    if (!_edgeDevice.get().stillAvailable()) {
        emit logMessage(FlasherTask::name() + ": edge disconnected",
                        AbstractSubtask::Error);
        emit flashingFinished(AbstractSubtask::Failed);
        return;
    }

    qInfo() << "Start flashing";

    if (!_edgeDevice.present()) {
        qFatal("edge device not present, but flasher executed.");
        emit flashingFinished(AbstractSubtask::Failed);
        return;
    }

    auto edgeAsStorage = _edgeDevice.get().asStorageDevice();

    qInfo() << "set flasher data";

    auto succeed = _flasherData.reset(firmwareFilename, edgeAsStorage);
    if (!succeed) {
        qFatal("Previous flasher data not released.");
        return;
    }


    qInfo() << "device" << _flasherData.device()->fileName() << " successfully opened";

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


