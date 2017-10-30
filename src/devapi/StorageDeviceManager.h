#ifndef STORAGEDEVICESCANNER_H
#define STORAGEDEVICESCANNER_H

#include <QObject>
#include <QtCore>
#include <memory>

#include "StorageDevice.h"
#include "ExecutionStatus.h"


class StorageDeviceManager : public QObject
{
    Q_OBJECT
public:
    static std::unique_ptr<StorageDeviceManager> instance();

    virtual QVector<std::shared_ptr<StorageDevice>> physicalDrives(int vid = -1, int pid = -1) = 0;

protected:
    StorageDeviceManager(QObject *parent = nullptr);
};

#endif // STORAGEDEVICESCANNER_H
