#ifndef PARTITION_H
#define PARTITION_H

#include <QtCore>
#include <memory>

namespace devlib {
    class Partition;
    class Mountpoint;
    class StorageDeviceInfo;

    namespace impl {
        class Partition_Private;
    }
}

class devlib::Partition
{
    friend class devlib::StorageDeviceInfo;
    friend class devlib::Mountpoint;
public:
    Partition(Partition&&);
    Partition(Partition const&);
    Partition operator =(Partition&&);
    Partition operator =(Partition const&);
    ~Partition(void);

    QString info(void) const noexcept;
    QString filePath(void) const noexcept;
    QString label(void) const noexcept;
    devlib::Mountpoint mount(QString const& path);

    QList<devlib::Mountpoint> mountpoints(void) const;

private:
    Partition(QString const& filePath, QString const& fstype, QString const& label);
    std::unique_ptr<impl::Partition_Private> _pimpl;
};

#endif // PARTITION_H
