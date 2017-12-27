#include <libudev.h>
#include <blkid/blkid.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

#include "StorageDeviceFile.h"
#include "util.h"

using namespace devlib;

class impl::StorageDeviceFile_Private
{
public:
    constexpr int incorrectLinuxFlags(void) { return -1; }

    int qflagsToLinuxFlags(QIODevice::OpenMode mode)
    {
        using QFileFlag = QIODevice::OpenModeFlag;
        auto linuxFlags = 0;

        if (mode & QFileFlag::ReadWrite) {
            linuxFlags |= O_RDWR;
        } else {
            if (mode & QFileFlag::ReadOnly) {
                linuxFlags |= O_RDONLY;
            } else if (mode & QFileFlag::WriteOnly) {
                linuxFlags |= O_WRONLY;
            } else {
                return incorrectLinuxFlags();
            }
        }

        if (mode & QFileFlag::Append) {
            linuxFlags |= O_APPEND;
        }

        if (mode & QFileFlag::Unbuffered) {
            linuxFlags |= O_SYNC;
        }

        if (mode & QFileFlag::Truncate) {
            linuxFlags |= O_TRUNC;
        }

        return linuxFlags;
    }

    int _fd;
};


StorageDeviceFile::StorageDeviceFile(StorageDeviceInfo const& info)
    : QFile(info.filePath()), _pimpl(new impl::StorageDeviceFile_Private())
{ }


StorageDeviceFile::~StorageDeviceFile(void)
{
    this->close();
}


bool StorageDeviceFile::open(OpenMode mode)
{
    Q_ASSERT(_pimpl);

    if (mode == NotOpen) {
        QFile::setErrorString("empty openMode");
        return false;
    }

    auto linuxFlags = _pimpl->qflagsToLinuxFlags(mode);

    if (linuxFlags == _pimpl->incorrectLinuxFlags()) {
        QFile::setErrorString("incorrect open flags");
        return false;
    }

    auto filenameStdString = QFile::fileName().toStdString();
    auto rawFileName = filenameStdString.data();
    _pimpl->_fd = ::open(rawFileName, linuxFlags);


    if (_pimpl->_fd == -1) {
        qCritical() << "failed to open: " << QFile::fileName();
        dbg::debugLinuxError();
        return false;
    }

    if (!QFile::open(_pimpl->_fd, mode)) {
        qCritical() << QFile::fileName() << " - failed to open in QFile. Code "
                    << static_cast<int>(error());
        ::close(_pimpl->_fd);
        return false;
    }

    return true;
}


void StorageDeviceFile::close(void)
{
    Q_ASSERT(_pimpl);
    ::close(_pimpl->_fd);
}
