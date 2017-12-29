#include <windows.h>
#include <fcntl.h>
#include <io.h>
#include "Mountpoint.h"

using namespace devlib;


class impl::MntptLock_Private
{
public:
    MntptLock_Private(QString const& mntptName, bool autoRemount)
        : _mntptName(mntptName), _autoRemount(autoRemount),
          _mntptHandle(INVALID_HANDLE_VALUE)
    { }

    void setHandle(HANDLE mntptHandle) {
        _mntptHandle = mntptHandle;
    }

    QString _mntptName;
    HANDLE  _mntptHandle;
    bool    _autoRemount;
};


MntptLock::MntptLock(QString const& mntptName, bool autoRemount)
    : _pimpl(new impl::MntptLock_Private(mntptName, autoRemount))
{ }


MntptLock::MntptLock(MntptLock&& lock)
    : _pimpl(std::move(lock._pimpl))
{ }


MntptLock& MntptLock::operator =(MntptLock&& lock)
{
    _pimpl = std::move(lock._pimpl);
    return *this;
}


MntptLock::~MntptLock(void)
{
    if (_pimpl) release();
}


bool MntptLock::release(void)
{
    Q_ASSERT(_pimpl);
    return ::CloseHandle(_pimpl->_mntptHandle);
}


class impl::Mountpoint_Private
{
public:
    Mountpoint_Private(QString const& filePath)
        : _filePath(filePath)
    { }

    static HANDLE genericUmount(QString const& mntpt) {
        HANDLE hMountpoint = ::CreateFile(mntpt.toStdWString().data(),
                                          GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE,
                                          NULL, OPEN_EXISTING, 0, NULL);

        if (hMountpoint == INVALID_HANDLE_VALUE) {
            return INVALID_HANDLE_VALUE;
        }

        DWORD bytesreturned;
        bool successful = ::DeviceIoControl(hMountpoint, FSCTL_LOCK_VOLUME,
                                            NULL, 0, NULL, 0, &bytesreturned, NULL);

        if (!successful) {
            ::CloseHandle(hMountpoint);
            qWarning() << "Can not lock volume: " << mntpt;
            return INVALID_HANDLE_VALUE;
        }

        DWORD junk;
        successful = ::DeviceIoControl(hMountpoint, FSCTL_DISMOUNT_VOLUME,
                                       NULL, 0, NULL, 0, &junk, NULL);

        if (!successful) {
            ::CloseHandle(hMountpoint);
            qWarning() << "Can not umount volume: " << mntpt;
            return INVALID_HANDLE_VALUE;
        }

        qInfo() << "Volume: " << mntpt << "Successfully unmounted";

        return hMountpoint;
    }

    QString _filePath;
    bool    _autoRemount;
};


Mountpoint::Mountpoint(QString const& filePath)
    : _pimpl(new impl::Mountpoint_Private(filePath))
{ }


Mountpoint::Mountpoint(Mountpoint&& mntpt) noexcept
    : _pimpl(std::move(mntpt._pimpl))
{ }


Mountpoint::Mountpoint(Mountpoint const& mntpt)
    : _pimpl(new impl::Mountpoint_Private(*mntpt._pimpl))
{ }


Mountpoint& Mountpoint::operator =(Mountpoint&& mntpt) noexcept
{
    _pimpl = std::move(mntpt._pimpl);
    return *this;
}


Mountpoint& Mountpoint::operator =(Mountpoint const& mntpt)
{
    _pimpl.reset(new impl::Mountpoint_Private(*mntpt._pimpl));
    return *this;
}


Mountpoint::~Mountpoint(void)
{ }


QString Mountpoint::info(void) const noexcept
{
    Q_ASSERT(_pimpl);
    return "";
}


QString Mountpoint::filePath(void) const noexcept
{
    Q_ASSERT(_pimpl);
    return _pimpl->_filePath;
}


bool Mountpoint::isValid(void) const noexcept
{
    Q_ASSERT(_pimpl);
    auto toMountpointPath = [] (QString const& volumeRootPath) {
        return QString(volumeRootPath).prepend("\\\\.\\").replace('/', "");
    };

    auto mntpts = QStorageInfo::mountedVolumes();

    auto predicate = [this, &toMountpointPath] (auto& arg) {
        return toMountpointPath(arg.rootPath()) == _pimpl->_filePath;
    };

    return std::any_of(mntpts.cbegin(), mntpts.cend(), predicate);
}


MntptLock Mountpoint::umount(bool autoRemount)
{
    Q_ASSERT(_pimpl);

    auto handle = impl::Mountpoint_Private::genericUmount(_pimpl->_filePath);
    auto lock   = MntptLock(_pimpl->_filePath, autoRemount);
    lock._pimpl->setHandle(handle);

    return lock;
}


MntptLock Mountpoint::umount(QString const& mntpt, bool autoRemount)
{
    auto handle = impl::Mountpoint_Private::genericUmount(mntpt);
    auto lock   = MntptLock(mntpt, autoRemount);
    lock._pimpl->setHandle(handle);

    return lock;
}
