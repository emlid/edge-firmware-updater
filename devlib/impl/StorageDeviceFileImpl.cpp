#include "StorageDeviceFileImpl.h"


devlib::impl::StorageDeviceFileImpl::
    StorageDeviceFileImpl(QString const& deviceFilename,
                          std::shared_ptr<IStorageDeviceInfo> storageDeviceInfo)
    : _deviceFilename(deviceFilename),
      _deviceInfo(std::move(storageDeviceInfo))
{ }


bool devlib::impl::StorageDeviceFileImpl::open_core(OpenMode mode)
{
    Q_UNUSED(mode);
    // first: unmount all mounpoints
    auto mntpts = _deviceInfo->mountpoints();
    _mntptsLocks.clear();

    for (auto const& mntpt : mntpts) {
        _mntptsLocks.push_back(mntpt->umount());
    }

    // second: open file handle
    _fileHandle = native::io::open(_deviceFilename.toStdString().data());

    QFile::setOpenMode(mode);

    return _fileHandle != nullptr;
}


void devlib::impl::StorageDeviceFileImpl::close_core(void)
{
    QFile::setOpenMode(QIODevice::NotOpen);
    _fileHandle.reset();
    _mntptsLocks.clear();
}


auto devlib::impl::StorageDeviceFileImpl::
    readData_core(char* data, qint64 len) -> qint64
{
    if (_fileHandle) {
        return native::io::read(_fileHandle.get(), data, len);
    } else {
        qWarning() << "StorageDevice not open";
        return 0;
    }
}


auto devlib::impl::StorageDeviceFileImpl::
    writeData_core(const char *data, qint64 len) -> qint64
{
    if (_fileHandle) {
        return native::io::write(_fileHandle.get(), data, len);
    } else {
        qWarning() << "StorageDevice not open";
        return 0;
    }
}


bool devlib::impl::StorageDeviceFileImpl::seek_core(qint64 pos)
{
    if (_fileHandle) {
        return native::io::seek(_fileHandle.get(), pos);
    } else {
        qWarning() << "StorageDevice not open";
        return false;
    }
}


void devlib::impl::StorageDeviceFileImpl::sync_core(void)
{
    devlib::native::io::sync(_fileHandle.get());
}
