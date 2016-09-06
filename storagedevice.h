#ifndef STORAGEDEVICE_H
#define STORAGEDEVICE_H

#include <QObject>
#include <QThread>

class StorageDevice;

class StorageDeviceThreadWorker: public QObject
{
    Q_OBJECT

public:
     StorageDeviceThreadWorker(StorageDevice* device);
     ~StorageDeviceThreadWorker(){}


signals:
     void updateProgress(int curr, int total);
     void flashComplete(void);

private slots:
     void _flash();
     void _updateProgress(int curr, int total) { emit updateProgress(curr, total); }
     void _cancel(void);


private:
    //StorageDeviceFlasher*   _flasher;
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
    //internal signals to communicate with thread
signals:
    void _flashOnThread(void);
    void _cancel(void);
    void flashComplete(void);
    void updateProgress(int curr, int total);
    void status(const QString& statusText);

public slots:
    //void _error(const QString& errorString) { emit error(errorString); }
    void _flashComplete(void) { emit flashComplete(); }
    void _updateProgress(int curr, int total) { emit updateProgress(curr, total); }

private:
    uint32_t _vid;
    uint32_t _pid;
    QString _deviceNode;

    StorageDeviceThreadWorker*  _worker;
    QThread*                    _workerThread;
};

#endif // STORAGEDEVICE_H
