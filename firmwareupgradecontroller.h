#ifndef FIRMWAREUPGRADECONTROLLER_H
#define FIRMWAREUPGRADECONTROLLER_H

#include <QObject>
#include <storagedevice.h>

class FirmwareUpgradeController;

class DeviceSearcher : public QObject
{
    Q_OBJECT

public:
    DeviceSearcher(FirmwareUpgradeController* parent);
    ~DeviceSearcher(){}

signals:
    void foundDevice(uint32_t vid, uint32_t pid, QString systemLocation);

public slots:
    void startFindBoardLoop();
};


class FirmwareUpgradeController : public QObject
{
    Q_OBJECT
public:
    explicit FirmwareUpgradeController(QObject *parent = 0);
    ~FirmwareUpgradeController();

    void startFindDevices();
    void clearDeviceList() {_connectedDevices.clear();}

signals:
    void statFindBoardLoop();

public slots:
    void addDevice(uint32_t vid, uint32_t pid, QString node);
private:
    QList<StorageDevice*> _connectedDevices;
    DeviceSearcher* searchWorker;
    QThread* searchWorkerThread;



};

#endif // FIRMWAREUPGRADECONTROLLER_H
