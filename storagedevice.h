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
     void updateProgress(uint32_t curr, uint32_t total);
     void flashComplete(void);
     void deviceWorkerMessage(const QString& text, bool critical);

private slots:
     void _flash(QString fileName);
     void _updateProgress(uint32_t curr, uint32_t total) { emit updateProgress(curr, total); }
     void _cancel(void);
     void deviceWorkerLog(const QString& text, bool critical = 0) {emit deviceWorkerMessage(text, critical);}


private:
    StorageDeviceFlasher*   _flasher;
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

    void setParams(uint32_t v, uint32_t p, QString d){_vid=v, _pid=p; _deviceNode=d;}
    QString getNode(){ return _deviceNode;}


    //internal signals to communicate with thread
signals:
    void _flashOnThread(QString fileName);
    void _cancel(void);
    void flashComplete(void);
    void updateProgress(uint32_t curr, uint32_t total);
    void status(const QString& statusText);
    void deviceMessage(const QString& text, bool critical = 0);

public slots:
    //void _error(const QString& errorString) { emit error(errorString); }
    void _flashComplete(void) { emit flashComplete(); }
    void _updateProgress(uint32_t curr, uint32_t total) { emit updateProgress(curr, total); }
    void flash(QString fileName) {emit _flashOnThread(fileName);}
    void deviceLog(const QString& text, bool critical = 0) {emit deviceMessage(text, critical);}
    void cancel() {emit _cancel();}

private:
    uint32_t _vid;
    uint32_t _pid;
    QString _deviceNode;

    StorageDeviceThreadWorker*  _worker;
    QThread*                    _workerThread;
};

#endif // STORAGEDEVICE_H
