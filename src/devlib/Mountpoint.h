#ifndef MOUNTPOINT_H
#define MOUNTPOINT_H

#include <QtCore>
#include <memory>

namespace devlib {
    class Mountpoint;
    class Partition;
    class MntptLock;
    class StorageDeviceInfo;

    namespace impl {
        class Mountpoint_Private;
        class MntptLock_Private;
    }
}

class devlib::Mountpoint
{
    friend class devlib::Partition;
    friend class devlib::StorageDeviceInfo;
    friend class devlib::MntptLock;
public:
    Mountpoint(Mountpoint&&) noexcept;
    Mountpoint(Mountpoint const&);
    Mountpoint& operator =(Mountpoint&&) noexcept;
    Mountpoint& operator =(Mountpoint const&);
    ~Mountpoint(void);

    QString info(void) const noexcept;
    bool    isValid(void) const noexcept;
    QString filePath(void) const noexcept;
    devlib::MntptLock umount(bool autoRemount = true);

    static devlib::MntptLock umount(QString const& mntpt, bool autoRemount = true);

private:
    Mountpoint(QString const& mntptPath);
    std::unique_ptr<impl::Mountpoint_Private> _pimpl;
};

class devlib::MntptLock
{
    friend class devlib::StorageDeviceInfo;
    friend class devlib::Mountpoint;
public:
    MntptLock(MntptLock&&);
    MntptLock& operator =(MntptLock&&);
    ~MntptLock(void);

    bool release(void);

private:
    MntptLock(QString const& mntptName, bool autoRemount);
    std::unique_ptr<impl::MntptLock_Private> _pimpl;
};

#endif // MOUNTPOINT_H
