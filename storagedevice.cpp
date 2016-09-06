#include "storagedevice.h"

int StorageDevice::deviceCount = 0;

StorageDeviceThreadWorker::StorageDeviceThreadWorker(StorageDevice* device):
    _deviceToFlash(device)
{
    Q_ASSERT(_deviceToFlash);

    connect(_deviceToFlash, &StorageDevice::_flashOnThread,  this, &StorageDeviceThreadWorker::_flash);
    connect(_deviceToFlash, &StorageDevice:: _cancel,  this, &StorageDeviceThreadWorker::_cancel, Qt::DirectConnection);
}

void StorageDeviceThreadWorker::_flash()
{
    //flashing...
    emit flashComplete();
}

void StorageDeviceThreadWorker::_cancel()
{
    //cancel
}



StorageDevice::StorageDevice(QObject *parent) : QObject(parent)
{
    deviceCount++;
    _worker = new StorageDeviceThreadWorker(this);
    Q_CHECK_PTR(_worker);

    _workerThread = new QThread(this);
    Q_CHECK_PTR(_workerThread);
    _worker->moveToThread(_workerThread);

    connect(_worker, &StorageDeviceThreadWorker::updateProgress, this, &StorageDevice::_updateProgress);
    connect(_worker, &StorageDeviceThreadWorker::flashComplete, this, &StorageDevice::_flashComplete);

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
