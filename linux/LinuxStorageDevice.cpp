#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>


#include "LinuxStorageDevice.h"


LinuxStorageDevice::LinuxStorageDevice(void)
    : _vid(-1),
      _pid(-1),
      _fd(-1),
      _blockSize(-1),
      _diskPath(""),
      _mountpoints()
{ }


LinuxStorageDevice::LinuxStorageDevice(int vid, int pid, int blockSize,
                                           QString const& diskPath,
                                           QVector<QString> const& mountpoints)
    : _vid(vid),
      _pid(pid),
      _fd(-1),
      _blockSize(blockSize),
      _diskPath(diskPath),
      _mountpoints(mountpoints)
{ }


LinuxStorageDevice::~LinuxStorageDevice(void)
{
    qDebug() << "Linux storage device released";
    close();
    remountAllMountpoints();
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
    return ::close(fd) != -1 ?
        ExecutionStatus::SUCCESS : ExecutionStatus(errno);
}


ExecutionStatus LinuxStorageDevice::unmountAllMountpoints(void)
{
    return ExecutionStatus::SUCCESS;
}


ExecutionStatus LinuxStorageDevice::unmount(QString const& mountpoint)
{
    return ExecutionStatus::SUCCESS;
}


ExecutionStatus LinuxStorageDevice::remount(QString const& mountpoint)
{
    return ExecutionStatus::SUCCESS;
}


ExecutionStatus LinuxStorageDevice::remountAllMountpoints(void)
{
    return ExecutionStatus::SUCCESS;
}


QString LinuxStorageDevice::toString() const
{
    QString info("");

    QTextStream(&info)
         << "Vid: "          << _vid << '\n'
         << "Pid: "          << _pid << '\n'
         << "Device path: "  << _devicePath << '\n'
         << "Block Size: "   << _blockSize << '\n'
         << "Mountpoints:";

    for (QString pt : _mountpoints) {
       QTextStream(&info) << pt << ", ";
    }

    QTextStream(&info) << '\n';

    return info;
}
