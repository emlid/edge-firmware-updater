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
    void noDeviceFound();
    void searchFinished();
    void searcherMessage(const QString& text, bool critical = 0);

public slots:
    void startFindBoardLoop();
    void prepareBoards();
};


class FirmwareUpgradeController : public QObject
{
    Q_OBJECT
public:
    explicit FirmwareUpgradeController(QObject *parent = 0);
    ~FirmwareUpgradeController();

    void startFindDevices();
    void clearDeviceList();
    QList<StorageDevice*> getDevices() {return _connectedDevices;}
    void flash(int selectedDeviceIndex, QString fileName);
    void cancel(int selectedDeviceIndex);
    bool flashingInProgress = 0;

signals:
    void findBoard();
    void deviceSearchFinished();
    void logMessage(const QString& text, bool critical = 0);
    void updateDeviceList();
    void _updateProgress(uint32_t bytesSent, uint32_t fileSize);
    void changeControlButtonsState();

public slots:
    void startFindBoardLoop() {emit findBoard();}
    void addDevice(uint32_t vid, uint32_t pid, QString node);
    void appendStatus(const QString& text, bool critical = 0) {emit logMessage(text, critical);}
    void searchFinished() {emit updateDeviceList(); emit deviceSearchFinished();}
    void updateProgress(uint32_t bytesSent, uint32_t fileSize) {emit _updateProgress(bytesSent, fileSize);}
    void flashingStoped() {flashingInProgress = 0; emit changeControlButtonsState();}
    void flashingStarted() {flashingInProgress = 1; emit changeControlButtonsState();}

private:
    QList<StorageDevice*> _connectedDevices;
    DeviceSearcher* searchWorker;
    QThread* searchWorkerThread;



};

#endif // FIRMWAREUPGRADECONTROLLER_H
