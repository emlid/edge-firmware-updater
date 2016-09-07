#ifndef FIRMWAREUPGRADECONTROLLER_H
#define FIRMWAREUPGRADECONTROLLER_H

#include <QObject>
#include <storagedevice.h>

#define VENDOR_ID "0a5c"

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
    void findBoard();
    void logMessage(const QString& text, bool critical = 0);

public slots:
    void startFindBoardLoop() {emit findBoard();}
    void addDevice(uint32_t vid, uint32_t pid, QString node);
    void appendStatus(const QString& text, bool critical = 0) {emit logMessage(text, critical);}

private:
    QList<StorageDevice*> _connectedDevices;
    DeviceSearcher* searchWorker;
    QThread* searchWorkerThread;



};

#endif // FIRMWAREUPGRADECONTROLLER_H
