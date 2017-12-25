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
    int qflagsToLinuxFlags(QIODevice::OpenMode mode)
    {
        using QFileFlag = QIODevice::OpenModeFlag;
        auto linuxFlags = 0;
        if (mode & QFileFlag::ReadOnly) {
            linuxFlags &= O_RDONLY;
        }

        if (mode & QFileFlag::WriteOnly) {
            linuxFlags &= O_WRONLY;
        }

        if (mode & QFileFlag::ReadWrite) {
            linuxFlags &= O_RDWR;
        }

        if (mode & QFileFlag::Append) {
            linuxFlags &= O_APPEND;
        }

        if (mode & QFileFlag::Unbuffered) {
            linuxFlags &= O_SYNC;
        }

        if (mode & QFileFlag::Truncate) {
            linuxFlags &= O_TRUNC;
        }

        return linuxFlags;
    }

    int _fd;
};


StorageDeviceFile::StorageDeviceFile(StorageDeviceInfo const& info)
    : QFile(info.filePath())
{ }


StorageDeviceFile::~StorageDeviceFile(void)
{
    this->close();
}


bool StorageDeviceFile::open(OpenMode mode)
{
    auto linuxFlags = _pimpl->qflagsToLinuxFlags(mode);

    auto rawFileName = fileName().toStdString().data();
    _pimpl->_fd = ::open(rawFileName, linuxFlags);

    if (_pimpl->_fd == -1) {
        dbg::qLinuxCritical() << fileName() << " - failed to open.";
        return false;
    }

    if (QFile::open(_pimpl->_fd, mode)) {
        qCritical() << fileName() << " - failed to open in QFile.";
        ::close(_pimpl->_fd);
        return false;
    }

    return true;
}


void StorageDeviceFile::close(void)
{
    ::close(_pimpl->_fd);
}
