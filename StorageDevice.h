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

    virtual int vid(void) const = 0;
    virtual int pid(void) const = 0;
    virtual QString diskPath(void) const = 0;
    virtual long recommendedBlockSize(void) const = 0;

    virtual ExecutionStatus open(int* const filedesc) = 0;
    virtual ExecutionStatus close(void) = 0;

    virtual QString toString(void) const = 0;
    virtual QVector<QString> mountpoints(void) const = 0;

    virtual ExecutionStatus unmountAllMountpoints(void) = 0;
    virtual ExecutionStatus unmount(QString const& mountpoint) = 0;

protected:
    StorageDevice(QObject *parent = nullptr);
    StorageDevice(int vid, int pid, QObject *parent = nullptr);
};

#endif // STORAGEDEVICE_H
