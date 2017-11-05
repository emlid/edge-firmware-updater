#ifndef STORAGEDEVICE_H
#define STORAGEDEVICE_H

#include <QObject>
#include <QtCore>

#include "ExecutionStatus.h"

class StorageDevice : public QObject
{
    Q_OBJECT
public:
    virtual ~StorageDevice(void) {}

    virtual int vid(void) const;
    virtual int pid(void) const;
    virtual long recommendedBlockSize(void) const;
    virtual ExecutionStatus openAsQFile(QFile* const qfile);

    virtual QString diskPath(void) const = 0;

    virtual ExecutionStatus open(int* const filedesc) = 0;
    virtual ExecutionStatus close(void) = 0;

    virtual ExecutionStatus unmountAllMountpoints(void) = 0;
    virtual ExecutionStatus unmount(QString const& mountpoint) = 0;

    virtual QString toString(void) const = 0;
    virtual QVector<QString> mountpoints(void) const = 0;


protected:
    StorageDevice(QObject *parent = nullptr);
    StorageDevice(int vid, int pid, long blockSize, QObject *parent = nullptr);

    int _vid;
    int _pid;
    long _blockSize;
};

#endif // STORAGEDEVICE_H