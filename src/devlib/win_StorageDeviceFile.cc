#include <windows.h>
#include <fcntl.h>
#include <io.h>

#include "StorageDeviceFile.h"
#include "util.h"

using namespace devlib;

class impl::StorageDeviceFile_Private
{
public:
    StorageDeviceFile_Private(StorageDeviceInfo const& info)
        : _deviceInfo(info)
    { }

    constexpr long incorrectWindowsFlags(void) const { return -1; }

    int qflagsToWindowsFlags(QIODevice::OpenMode mode)
    {
        using QFileFlag = QIODevice::OpenModeFlag;
        auto windowsFlags = 0L;

            if (mode & QFileFlag::ReadOnly) {
                windowsFlags |= GENERIC_READ;
            }

            if (mode & QFileFlag::WriteOnly) {
                windowsFlags |= GENERIC_WRITE;
            }

        return windowsFlags == 0L ?
                    incorrectWindowsFlags() : windowsFlags;
    }

    QIODevice::OpenMode qflagsFilter(QIODevice::OpenMode mode) {
        QIODevice::OpenMode filteredMode = QIODevice::NotOpen;

        if (mode & QIODevice::ReadOnly) {
            filteredMode |= QIODevice::ReadOnly;
        }

        if (mode & QIODevice::WriteOnly) {
            filteredMode |= QIODevice::WriteOnly;
        }

        return filteredMode;
    }

    int                     _fd;
    HANDLE                  _handle;
    StorageDeviceInfo       _deviceInfo;
    std::vector<MntptLock>  _mntptLockPool;
};


StorageDeviceFile::StorageDeviceFile(StorageDeviceInfo const& info)
    : QFile(info.filePath()), _pimpl(new impl::StorageDeviceFile_Private(info))
{ }


StorageDeviceFile::~StorageDeviceFile(void)
{
    if (_pimpl) this->close();
}


bool StorageDeviceFile::open(OpenMode mode)
{
    Q_ASSERT(_pimpl);

    if (mode == NotOpen) {
        QFile::setErrorString("empty openMode");
        return false;
    }

    auto filenameStdString = QFile::fileName().toStdWString();
    auto rawFileName = filenameStdString.data();
    auto mntpts = _pimpl->_deviceInfo.mountpoints();

    for (auto& mntpt : mntpts) {
        _pimpl->_mntptLockPool.push_back(mntpt.umount());
    }

    auto winFlags = _pimpl->qflagsToWindowsFlags(mode);

    if (winFlags == _pimpl->incorrectWindowsFlags()) {
        QFile::setErrorString("incorrect open flags");
        dbg::debugWindowsError();
        return false;
    }

    _pimpl->_handle = ::CreateFile(rawFileName, GENERIC_READ | GENERIC_WRITE,
                                   FILE_SHARE_READ | FILE_SHARE_WRITE,
                                   NULL, OPEN_EXISTING, 0, NULL);

    if (_pimpl->_handle == INVALID_HANDLE_VALUE) {
        QFile::setErrorString("can not open");
        dbg::debugWindowsError();
        return false;
    }

    _pimpl->_fd = _open_osfhandle(reinterpret_cast<intptr_t>(_pimpl->_handle), _O_RAW | _O_RDWR);

    if (!QFile::open(_pimpl->_fd, QIODevice::ReadWrite | QIODevice::Truncate | QIODevice::Unbuffered)) {
        qCritical() << QFile::fileName() << " - failed to open in QFile. Code "
                    << static_cast<int>(error());
        close();
        return false;
    }

    return true;
}


void StorageDeviceFile::close(void)
{
    Q_ASSERT(_pimpl);
    if (_pimpl->_handle != INVALID_HANDLE_VALUE
        && !::CloseHandle(_pimpl->_handle))
    {
        dbg::debugWindowsError();
    }

    _pimpl->_mntptLockPool.clear();
}
