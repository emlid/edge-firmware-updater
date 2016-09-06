#include "firmwareupgradecontroller.h"
#include <QDebug>

DeviceSearcher::DeviceSearcher(FirmwareUpgradeController *parent)
{
    connect(parent, &FirmwareUpgradeController::statFindBoardLoop, this, &DeviceSearcher::startFindBoardLoop, Qt::ConnectionType::DirectConnection);
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

    searchWorkerThread->start();
}

FirmwareUpgradeController::~FirmwareUpgradeController(){
    searchWorkerThread->quit();
    searchWorkerThread->wait();

    delete searchWorkerThread;
}

void FirmwareUpgradeController::startFindDevices()
{
    qDebug() << "start Find defices";
}


void FirmwareUpgradeController::addDevice(uint32_t vid, uint32_t pid, QString node)
{
    StorageDevice* dev = new StorageDevice(this->parent());
    dev->setParams(vid, pid, node);
    _connectedDevices.append(dev);
}
