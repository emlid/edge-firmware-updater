#include "firmwareupgradecontroller.h"
#include <QDebug>

DeviceSearcher::DeviceSearcher(FirmwareUpgradeController *parent)
{
    connect(parent, &FirmwareUpgradeController::findBoard, this, &DeviceSearcher::startFindBoardLoop, Qt::ConnectionType::DirectConnection);
}

FirmwareUpgradeController::FirmwareUpgradeController(QObject *parent) : QObject(parent)
{
    qRegisterMetaType<uint32_t>("uint32_t");
    searchWorker = new DeviceSearcher(this);
    Q_CHECK_PTR(searchWorker);

    searchWorkerThread = new QThread(this);
    Q_CHECK_PTR(searchWorkerThread);
    searchWorker->moveToThread(searchWorkerThread);

    connect(searchWorker, &DeviceSearcher::foundDevice, this, &FirmwareUpgradeController::addDevice);
    connect(searchWorker, &DeviceSearcher::searchFinished, this, &FirmwareUpgradeController::searchFinished);
    connect(searchWorker, &DeviceSearcher::searcherMessage, this, &FirmwareUpgradeController::appendStatus);

    searchWorkerThread->start();
}

FirmwareUpgradeController::~FirmwareUpgradeController(){
    searchWorkerThread->quit();
    searchWorkerThread->wait();

    delete searchWorkerThread;
    delete searchWorker;
}


void FirmwareUpgradeController::addDevice(uint32_t vid, uint32_t pid, QString node)
{
    StorageDevice* dev = new StorageDevice(this->parent());
    dev->setParams(vid, pid, node);
    _connectedDevices.append(dev);

    connect(_connectedDevices.last(), &StorageDevice::updateProgress, this, &FirmwareUpgradeController::updateProgress);
    connect(_connectedDevices.last(), &StorageDevice::deviceMessage, this, &FirmwareUpgradeController::appendStatus);
}


void FirmwareUpgradeController::flash(int selectedDeviceIndex, QString fileName)
{
     _connectedDevices[selectedDeviceIndex]->flash(fileName);
}

void FirmwareUpgradeController::cancel(int selectedDeviceIndex)
{
    _connectedDevices[selectedDeviceIndex]->cancel();
}
