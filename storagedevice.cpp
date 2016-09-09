#include "storagedevice.h"
#include <QDebug>

int StorageDevice::deviceCount = 0;

StorageDeviceThreadWorker::StorageDeviceThreadWorker(StorageDevice* device):
    _deviceToFlash(device)
{
    Q_ASSERT(_deviceToFlash);

    connect(_deviceToFlash, &StorageDevice::_flashOnThread,  this, &StorageDeviceThreadWorker::_flash);
    connect(_deviceToFlash, &StorageDevice:: _cancel,  this, &StorageDeviceThreadWorker::_cancel, Qt::DirectConnection);

    _flasher = new StorageDeviceFlasher(this);
    connect(_flasher, &StorageDeviceFlasher::updateProgress, this, &StorageDeviceThreadWorker::_updateProgress);
}

void StorageDeviceThreadWorker::_flash(QString fileName)
{
     struct FlashingParameters testParams;

     testParams.inputFile.append(fileName);
     testParams.outputFile.append(QString("%1").arg(_deviceToFlash->getNode()));

     emit deviceWorkerLog(QString("Flashing %1 with %2").arg(testParams.outputFile).arg(testParams.inputFile));

     int ret = _flasher->flashDevice(testParams);
     if (ret) {
         emit deviceWorkerLog(QString("Failed to Flash: %1").arg(ret), true);
     } else {
         emit deviceWorkerLog("Flashing complete");
     }

    emit flashComplete();
}

void StorageDeviceThreadWorker::_cancel()
{
    _flasher->terminate();
    _flasher->deleteLater();

}



StorageDevice::StorageDevice(QObject *parent) : QObject(parent)
{
    qDebug() << "ctor with parent";
    qDebug() << "this->parent:" << this->parent()->objectName();
    deviceCount++;
    _worker = new StorageDeviceThreadWorker(this);
    Q_CHECK_PTR(_worker);

    _workerThread = new QThread(this);
    Q_CHECK_PTR(_workerThread);
    _worker->moveToThread(_workerThread);

    connect(_worker, &StorageDeviceThreadWorker::updateProgress, this, &StorageDevice::_updateProgress);
    connect(_worker, &StorageDeviceThreadWorker::flashComplete, this, &StorageDevice::_flashComplete);
    connect(_worker, &StorageDeviceThreadWorker::deviceWorkerMessage, this, &StorageDevice::deviceLog);

    _workerThread->start();
}

StorageDevice::~StorageDevice()
{
    _workerThread->quit();
    _workerThread->wait();

    delete _workerThread;
}

StorageDevice& StorageDevice::operator=(const StorageDevice& other){

    if (this != &other) {
        this->_vid = other._vid;
        this->_pid = other._pid;
        this->_deviceNode = other._deviceNode;
    }
    return *this;
}
