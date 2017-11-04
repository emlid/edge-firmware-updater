#include <sys/types.h>
#include <sys/mount.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>


#include "LinuxStorageDevice.h"


LinuxStorageDevice::LinuxStorageDevice(void)
    : _vid(-1),
      _pid(-1),
      _blockSize(-1),
      _diskPath(""),
      _mountpoints(),
      _fd(-1)
{ }


LinuxStorageDevice::LinuxStorageDevice(int vid, int pid, int blockSize,
                                           QString const& diskPath,
                                           QVector<QString> const& mountpoints)
    : _vid(vid),
      _pid(pid),
      _blockSize(blockSize),
      _diskPath(diskPath),
      _mountpoints(mountpoints),
      _fd(-1)
{ }


LinuxStorageDevice::~LinuxStorageDevice(void)
{
    qDebug() << "Linux storage device released";
    close();
}


int LinuxStorageDevice::vid(void) const
{
    return _vid;
}


int LinuxStorageDevice::pid(void) const
{
    return _pid;
}


long LinuxStorageDevice::recommendedBlockSize(void) const
{
    return _blockSize;
}


QString LinuxStorageDevice::diskPath(void) const
{
    return _diskPath;
}


QVector<QString> LinuxStorageDevice::mountpoints() const
{
    return _mountpoints;
}


ExecutionStatus LinuxStorageDevice::open(int* const filedesc)
{
    _fd = ::open(_diskPath.toStdString().data(), O_WRONLY);
    if (_fd == -1) {
        return ExecutionStatus(errno, _diskPath + ": can not open file");
    }

    (*filedesc) = _fd;

    return ExecutionStatus::SUCCESS;
}


ExecutionStatus LinuxStorageDevice::close(void)
{
    return ::close(_fd) != -1 ?
        ExecutionStatus::SUCCESS : ExecutionStatus(errno);
}


ExecutionStatus LinuxStorageDevice::
    unmountAllMountpoints(void)
{
    for (QString const& mntpt : _mountpoints) {
        auto status = unmount(mntpt);
        if (status.failed()) {
            return status;
        }
    }

    return ExecutionStatus::SUCCESS;
}


ExecutionStatus LinuxStorageDevice::
    unmount(QString const& mountpoint)
{
    if (::umount(mountpoint.toStdString().data())) {
        return ExecutionStatus(errno, mountpoint + ": unmounting failed");
    }

    return ExecutionStatus::SUCCESS;
}


QString LinuxStorageDevice::toString() const
{
    QString info("");

    QTextStream(&info)
         << "Vid: "          << _vid << '\n'
         << "Pid: "          << _pid << '\n'
         << "Device path: "  << _diskPath << '\n'
         << "Block Size: "   << _blockSize << '\n'
         << "Mountpoints:";

    for (QString const& pt : _mountpoints) {
       QTextStream(&info) << pt << ", ";
    }

    QTextStream(&info) << '\n';

    return info;
}
