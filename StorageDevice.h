#ifndef STORAGEDEVICE_H
#define STORAGEDEVICE_H

#include <QObject>
#include <QtCore>

#include "ExitStatus.h"

class StorageDevice : public QObject
{
    Q_OBJECT
public:

    virtual int vid(void) = 0;
    virtual int pid(void) = 0;
    virtual QString diskPath(void) = 0;
    virtual ulong recommendedBlockSize(void) = 0;

    virtual ExitStatus open(int* const filedesc) = 0;
    virtual QString toString(void) = 0;
    virtual QVector<QString> mountpoints(void) = 0;

    virtual ExitStatus unmountAllMountpoints(void) = 0;
    virtual ExitStatus unmount(QString const& mountpoint) = 0;

protected:
    StorageDevice(QObject *parent = nullptr);
    StorageDevice(int vid, int pid, QObject *parent = nullptr);
};

#endif // STORAGEDEVICE_H
