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
            qWarning() << "can not unmount: " << mntpt;
            dbg::debugLinuxError();
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
    auto info = QString();

    QTextStream(&info)
            << fmt::beg() << " name: " << _pimpl->_filePath << " " << fmt::end();

    return info;
}


bool Mountpoint::isValid(void) const noexcept
{
    Q_ASSERT(_pimpl);
    auto mntpts = QStorageInfo::mountedVolumes();

    auto predicate = [this] (auto& arg) {
        return arg.rootPath() == _pimpl->_filePath;
    };

    return std::any_of(mntpts.cbegin(), mntpts.cend(), predicate);
}


QString Mountpoint::filePath(void) const noexcept
{
    Q_ASSERT(_pimpl);
    return _pimpl->_filePath;
}


MntptLock Mountpoint::umount(bool autoRemount)
{
    Q_ASSERT(_pimpl);

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
    : _pimpl((std::move(lock._pimpl)))
{ }


MntptLock& MntptLock::operator =(MntptLock&& lock)
{
    _pimpl = std::move(lock._pimpl);
    return *this;
}


MntptLock::~MntptLock(void)
{
    if (_pimpl && _pimpl->_autoRemount) {
        release();
    }
}


bool MntptLock::release(void)
{
    Q_ASSERT(_pimpl);
    // stub
    return true;
}
