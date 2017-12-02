#include "DeviceScannerSubtask.h"
#include "devapi/StorageDeviceManager.h"


DeviceScannerSubtask::DeviceScannerSubtask(int vid, QList<int> const& pids, QObject *parent)
    : AbstractSubtask(QStringLiteral("DeviceScannerSubtask"), parent), _vid(vid), _pids(pids)
{ }


DeviceScannerSubtask::~DeviceScannerSubtask(void)
{
    qDebug() << _subtaskMsg("finished");
}


void DeviceScannerSubtask::run(void)
{
    using DsState = states::DeviceScannerState;
    using StType  = states::StateType;

    auto const maxPollingTime = 10000;
    auto const sleepTime      = 600;
    auto totalSleepTime       = 0;

    auto manager = StorageDeviceManager::instance();

    // We need to wait while OS detect our device, which was booted as mass storage
    // Consequently, we need to wait while OS auto-mount device partitions

    emit stateChanged(DsState::ScannerStarted);
    qInfo() << _subtaskMsg("Device detecting...Polling..");

    do {
        QThread::msleep(sleepTime);
        auto drives = manager->physicalDrives(_vid, _pids.at(0));

        if (drives.size() > 0) {
            _physicalDrives = std::move(drives);
            break;
        }

        totalSleepTime += sleepTime;

        if (_stopRequested()) {
            emit stateChanged(DsState::ScannerCancelled);
            emit finished(false);
            return;
        }

    } while (totalSleepTime < maxPollingTime);

    if (totalSleepTime > maxPollingTime) {
        qCritical() << _subtaskMsg("Rpi as mass storage device not found");
        emit stateChanged(DsState::ScannerDeviceNotFound, StType::Error);
        emit finished(false);
        return;
    }

    qInfo() << _subtaskMsg("Rpi device's found.");
    emit stateChanged(DsState::ScannerDeviceFound);
    emit stateChanged(DsState::ScannerFinished);
    emit finished();
}
