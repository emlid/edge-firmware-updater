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
    int findBootableDevices();
    void startUdevMonitor(int count);
    void enumerateDevices();
};


class FirmwareUpgradeController : public QObject
{
    Q_OBJECT
public:
    explicit FirmwareUpgradeController(QObject *parent = 0);
    ~FirmwareUpgradeController();

    void clearDeviceList();
    QList<StorageDevice*> getDevices() {return _connectedDevices;}
    void flash(int selectedDeviceIndex, QString fileName);
    void cancel(int selectedDeviceIndex);
    bool flashingInProgress = false;

signals:
    void deviceSearchFinished();
    void logMessage(const QString& text, bool critical = 0);
    void updateDeviceList();
    void updateProgressBar(int newValue, int progressBarIndex);
    void changeControlButtonsState();
    void _findBoard();

public slots:
    void startFindBoardLoop() {emit _findBoard();}
    void updateProgress(uint32_t bytesSent, uint32_t fileSize, int deviceIndex);
    void flashingStopped();
    void flashingStarted();
    void _addDevice(uint32_t vid, uint32_t pid, QString node);
    void _appendStatus(const QString& text, bool critical = 0) {emit logMessage(text, critical);}
    void _searchFinished();

private:
    QList<StorageDevice*>   _connectedDevices;
    DeviceSearcher*         _searchWorker;
    QThread*                _searchWorkerThread;



};

#endif // FIRMWAREUPGRADECONTROLLER_H
