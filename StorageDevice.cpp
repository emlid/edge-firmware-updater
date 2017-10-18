#include "StorageDevice.h"

StorageDevice::StorageDevice(QObject *parent) : QObject(parent)
{  }


StorageDevice::StorageDevice(int vid, int pid, long blockSize, QObject *parent)
    : _vid(vid), _pid(pid), _blockSize(blockSize), QObject(parent)
{  }


int StorageDevice::vid(void) const
{
    return _vid;
}


int StorageDevice::pid(void) const
{
    return _pid;
}


long StorageDevice::recommendedBlockSize(void) const
{
    return _blockSize;
}


ExecutionStatus StorageDevice::openAsQFile(QFile * const qfile)
{
    int fd = 0;
    auto status = open(&fd);

    if (!status.failed()) {
        qfile->open(fd, QIODevice::ReadWrite);
    }

    return status;
}
