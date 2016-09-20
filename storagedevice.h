#ifndef STORAGEDEVICE_H
#define STORAGEDEVICE_H

#include <QObject>
#include <QThread>
#include <storagedeviceflasher.h>

class StorageDevice;

class StorageDeviceThreadWorker: public QObject
{
    Q_OBJECT

public:
     StorageDeviceThreadWorker(StorageDevice* device);
     ~StorageDeviceThreadWorker(){}


signals:
     void flashingStarted();
     void updateProgress(uint32_t curr, uint32_t total);
     void flashComplete(void);
     void deviceWorkerMessage(const QString& text, bool critical = 0);

private slots:
     void _startFlashing() {emit flashingStarted();}
     void _deviceWorkerLog(const QString& text, bool critical = 0) {emit deviceWorkerMessage(text, critical);}
     void _flash(QString fileName);
     void _updateProgress(uint32_t curr, uint32_t total) { emit updateProgress(curr, total); }
     void _cancel(void);

private:
    StorageDeviceFlasher*   _flasher = 0;
    StorageDevice* _deviceToFlash;
};




class StorageDevice : public QObject
{

    Q_OBJECT
public:
    StorageDevice(QObject *parent = 0);
    ~StorageDevice();

    StorageDevice &operator=(const StorageDevice &other);

    static int deviceCount;

    void setParams(uint32_t v, uint32_t p, QString d) {_vid=v, _pid=p; _deviceNode=d;}
    QString getNode() { return _deviceNode;}

signals:
    void flashingStarted();
    void flashComplete();
    void updateProgress(uint32_t curr, uint32_t total);
    void deviceMessage(const QString& text, bool critical = 0);
    void _flashOnThread(QString fileName);
    void _cancel();

public slots:
    void flash(QString fileName) {emit _flashOnThread(fileName);}
    void deviceLog(const QString& text, bool critical = 0) {emit deviceMessage(text, critical);}
    void cancel() {emit _cancel();}
    void _flashingStarted() {emit flashingStarted();}
    void _flashComplete() {emit flashComplete();}
    void _updateProgress(uint32_t curr, uint32_t total) {emit updateProgress(curr, total);}

private:
    uint32_t _vid;
    uint32_t _pid;
    QString  _deviceNode;

    StorageDeviceThreadWorker*  _worker;
    QThread*                    _workerThread;
};

#endif // STORAGEDEVICE_H
