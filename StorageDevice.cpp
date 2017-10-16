#include "StorageDevice.h"

StorageDevice::StorageDevice(QObject *parent) : QObject(parent)
{  }


ExecutionStatus StorageDevice::openAsQFile(QFile * const qfile)
{
    int fd = 0;
    auto status = open(&fd);

    if (!status.failed()) {
        qfile->open(fd, QIODevice::WriteOnly);
    }

    return status;
}
