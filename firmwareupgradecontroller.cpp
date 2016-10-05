#include "firmwareupgradecontroller.h"
#include <QDebug>

DeviceSearcher::DeviceSearcher(FirmwareUpgradeController *parent)
{
    connect(parent, &FirmwareUpgradeController::_findBoard, this, &DeviceSearcher::startFindBoardLoop);
}

FirmwareUpgradeController::FirmwareUpgradeController(QObject *parent) : QObject(parent)
{
    qRegisterMetaType<uint32_t>("uint32_t");
    _searchWorker = new DeviceSearcher(this);
    Q_CHECK_PTR(_searchWorker);

    _searchWorkerThread = new QThread(this);
    Q_CHECK_PTR(_searchWorkerThread);
    _searchWorker->moveToThread(_searchWorkerThread);

    connect(_searchWorker, &DeviceSearcher::foundDevice, this, &FirmwareUpgradeController::_addDevice);
    connect(_searchWorker, &DeviceSearcher::searchFinished, this, &FirmwareUpgradeController::_searchFinished);
    connect(_searchWorker, &DeviceSearcher::searcherMessage, this, &FirmwareUpgradeController::_appendStatus);

    _searchWorkerThread->start();
}

FirmwareUpgradeController::~FirmwareUpgradeController(){
    _searchWorkerThread->quit();
    _searchWorkerThread->wait();

    delete _searchWorkerThread;
    delete _searchWorker;
}


void FirmwareUpgradeController::_addDevice(uint32_t vid, uint32_t pid, QString node)
{
    StorageDevice* dev = new StorageDevice(this);
    dev->setParams(vid, pid, node);
    _connectedDevices.append(dev);

    connect(_connectedDevices.last(), &StorageDevice::updateProgress, this, &FirmwareUpgradeController::updateProgress);
    connect(_connectedDevices.last(), &StorageDevice::deviceMessage, this, &FirmwareUpgradeController::_appendStatus);
    connect(_connectedDevices.last(), &StorageDevice::flashComplete, this, &FirmwareUpgradeController::flashingStopped);
    connect(_connectedDevices.last(), &StorageDevice::flashingStarted, this, &FirmwareUpgradeController::flashingStarted);
}

void FirmwareUpgradeController::_searchFinished()
{
    emit updateDeviceList();
    emit deviceSearchFinished();
}

void FirmwareUpgradeController::flashingStopped()
{
    flashingInProgress = 0;
    emit changeControlButtonsState();
    emit changeListAndBarFocus();
}

void FirmwareUpgradeController::flashingStarted()
{
    flashingInProgress = 1;
    emit changeControlButtonsState();
}

void FirmwareUpgradeController::flash(int selectedDeviceIndex, QString fileName)
{
     _connectedDevices[selectedDeviceIndex]->flash(fileName);
}

void FirmwareUpgradeController::cancel(int selectedDeviceIndex)
{
    _connectedDevices[selectedDeviceIndex]->cancel();
}

void FirmwareUpgradeController::clearDeviceList()
{
    qDeleteAll(_connectedDevices.begin(), _connectedDevices.end());
    _connectedDevices.clear();
}
