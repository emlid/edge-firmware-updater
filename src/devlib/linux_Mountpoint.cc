#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mount.h>
#include <unistd.h>

#include <algorithm>

#include "Mountpoint.h"
#include "util.h"


using namespace devlib;


class impl::Mountpoint_Private
{
public:
    Mountpoint_Private(QString const& filePath)
        : _filePath(filePath)
    { }

    static bool genericUmount(QString const& mntpt) {
        auto mntptName = mntpt.toStdString();

        if (::umount2(mntptName.data(), 0) != 0) {
            dbg::qLinuxCritical() << mntpt << " - unmounting failed." ;
            return false;
        }

        qInfo() << mntpt << " - was unmounted";
        return true;
    }

    QString _filePath;
    bool    _autoRemount;
};


Mountpoint::Mountpoint(QString const& filePath)
    : _pimpl(new impl::Mountpoint_Private(filePath))
{ }


Mountpoint::Mountpoint(Mountpoint&& mntpt)
    : _pimpl(new impl::Mountpoint_Private(std::move(*mntpt._pimpl)))
{ }


Mountpoint::Mountpoint(Mountpoint const& mntpt)
    : _pimpl(new impl::Mountpoint_Private(*mntpt._pimpl))
{ }


Mountpoint Mountpoint::operator =(Mountpoint&& mntpt)
{
    return Mountpoint(std::move(mntpt));
}


Mountpoint Mountpoint::operator =(Mountpoint const& mntpt)
{
    return Mountpoint(mntpt);
}


Mountpoint::~Mountpoint(void)
{ }


QString Mountpoint::info(void) const noexcept
{
    auto info = QString();

    QTextStream(&info)
            << fmt::beg() << " name: " << _pimpl->_filePath << " " << fmt::end();

    return info;
}


bool Mountpoint::isValid(void) const noexcept
{
    auto mntpts = QStorageInfo::mountedVolumes();

    auto predicate = [this] (auto& arg) {
        return arg.rootPath() == _pimpl->_filePath;
    };

    return std::any_of(mntpts.cbegin(), mntpts.cend(), predicate);
}


QString Mountpoint::filePath(void) const noexcept
{
    return _pimpl->_filePath;
}


MntptLock Mountpoint::umount(bool autoRemount)
{
    impl::Mountpoint_Private::genericUmount(_pimpl->_filePath);
    return MntptLock(_pimpl->_filePath, autoRemount);
}


MntptLock Mountpoint::umount(QString const& mntpt, bool autoRemount)
{
    impl::Mountpoint_Private::genericUmount(mntpt);
    return MntptLock(mntpt, autoRemount);
}


class impl::MntptLock_Private
{
public:
    MntptLock_Private(QString const& mntptName, bool autoRemount)
        : _mntptName(mntptName), _autoRemount(autoRemount)
    { }

    QString _mntptName;
    bool    _autoRemount;
};


MntptLock::MntptLock(QString const& mntptName, bool autoRemount)
    : _pimpl(new impl::MntptLock_Private(mntptName, autoRemount))
{ }


MntptLock::MntptLock(MntptLock&& lock)
    : _pimpl(new impl::MntptLock_Private(std::move(*lock._pimpl)))
{ }


MntptLock MntptLock::operator =(MntptLock&& lock)
{
    return MntptLock(std::move(lock));
}


MntptLock::~MntptLock(void)
{
    if (_pimpl->_autoRemount) {
        release();
    }
}


bool MntptLock::release(void)
{
    // stub
    return true;
}
