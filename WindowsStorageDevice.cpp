#include "WindowsStorageDevice.h"

/* WinAPI headers*/
#include <windows.h>
#include <fcntl.h>
#include <io.h>

WindowsStorageDevice::WindowsStorageDevice()
    : _vid(-1),
      _pid(-1),
      _blockSize(-1),
      _driveNumber(-1),
      _devicePath(""),
      _mountpoints(),
      _unmountedMountpoints(),
      _handle(INVALID_HANDLE_VALUE)
{ }


WindowsStorageDevice::WindowsStorageDevice(int vid, int pid, int blockSize,
                                           int driveNumber, QString const& devicePath,
                                           QVector<QString> const& mountpoints)
    : _vid(vid),
      _pid(pid),
      _blockSize(blockSize),
      _driveNumber(driveNumber),
      _devicePath(devicePath),
      _mountpoints(mountpoints),
      _unmountedMountpoints(),
      _handle(INVALID_HANDLE_VALUE)
{ }


WindowsStorageDevice::~WindowsStorageDevice(void)
{
    qDebug() << "Windows storage device released";
    close();
    remountAllMountpoints();
}


int WindowsStorageDevice::vid(void) const
{
    return _vid;
}


int WindowsStorageDevice::pid(void) const
{
    return _pid;
}


ulong WindowsStorageDevice::recommendedBlockSize(void) const
{
    return _blockSize;
}


QString WindowsStorageDevice::diskPath(void) const
{
    return QString("\\\\.\\PhysicalDrive%1").arg(_driveNumber);
}


QString WindowsStorageDevice::devicePath(void) const
{
    return _devicePath;
}


QVector<QString> WindowsStorageDevice::mountpoints() const
{
    return _mountpoints;
}


ExitStatus WindowsStorageDevice::open(int* const filedesc)
{
    _handle = ::CreateFile(diskPath().toStdWString().data(), GENERIC_WRITE,
                               FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);

    if (_handle == INVALID_HANDLE_VALUE) {
        return ExitStatus(::GetLastError(), _devicePath + ": open for writing failed.");
    }

    *filedesc = _open_osfhandle((LONG)_handle, _O_RAW | _O_WRONLY);

    return ExitStatus::SUCCESS;
}


ExitStatus WindowsStorageDevice::close(void)
{
    if (!::CloseHandle(_handle)) {
        return ExitStatus(::GetLastError());
    }

    return ExitStatus::SUCCESS;
}


ExitStatus WindowsStorageDevice::unmountAllMountpoints(void)
{
    for (QString mntpt : _mountpoints) {
        ExitStatus status = unmount(mntpt);
        if (status.failed()) {
            return status;
        }
    }

    return ExitStatus::SUCCESS;
}


ExitStatus WindowsStorageDevice::unmount(QString const& mountpoint)
{
    HANDLE hMountpoint = ::CreateFile(mountpoint.toStdWString().data(),
                                      GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE,
                                      NULL, OPEN_EXISTING, 0, NULL);

    if (hMountpoint == INVALID_HANDLE_VALUE) {
        return ExitStatus(::GetLastError(), mountpoint + ": Can't open mountpoint.");
    }

    DWORD bytesreturned;
    bool succesful = ::DeviceIoControl(hMountpoint, FSCTL_LOCK_VOLUME,
                                       NULL, 0, NULL, 0, &bytesreturned, NULL);

    if (!succesful) {
        ::CloseHandle(hMountpoint);
        return ExitStatus(::GetLastError(), mountpoint + ": Can't get lock on mountpoint.");
    }

    DWORD junk;
    succesful = ::DeviceIoControl(hMountpoint, FSCTL_DISMOUNT_VOLUME,
                                  NULL, 0, NULL, 0, &junk, NULL);

    if (!succesful) {
        ::CloseHandle(hMountpoint);
        return ExitStatus(::GetLastError(), mountpoint + ": Can't unmount.");
    }

    _mountpoints.removeOne(mountpoint);
    _unmountedMountpoints.insert(mountpoint, hMountpoint);

    return ExitStatus::SUCCESS;
}


ExitStatus WindowsStorageDevice::remount(QString const& mountpoint)
{
    auto handle = _unmountedMountpoints.value(mountpoint);
    if (::CloseHandle(handle)) {
        _unmountedMountpoints.remove(mountpoint);
        return ExitStatus::SUCCESS;
    }
    return ExitStatus(::GetLastError());
}


ExitStatus WindowsStorageDevice::remountAllMountpoints(void)
{
    for (QString mntpt : _unmountedMountpoints.keys()) {
        auto result = remount(mntpt);
        if (result.failed()) {
            return result;
        }
    }

    return ExitStatus::SUCCESS;
}


QString WindowsStorageDevice::toString() const
{
    QString info("");

    QTextStream(&info)
         << "Vid: "          << _vid << '\n'
         << "Pid: "          << _pid << '\n'
         << "Device path: "  << _devicePath << '\n'
         << "Drive Number: " << _driveNumber << '\n'
         << "Block Size: "   << _blockSize << '\n'
         << "Mountpoints:";

    for (QString pt : _mountpoints) {
       QTextStream(&info) << pt << ", ";
    }

    QTextStream(&info) << '\n';

    return info;
}
