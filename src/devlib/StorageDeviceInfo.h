#ifndef STORAGEDEVICE_H
#define STORAGEDEVICE_H

#include <QtCore>
#include <memory>

#include "Mountpoint.h"
#include "Partition.h"

namespace devlib {
    class StorageDeviceInfo;
    namespace impl {
        class StorageDeviceInfo_Private;
    }
}

class devlib::StorageDeviceInfo
{
    friend class devlib::Mountpoint;
    friend class devlib::Partition;
public:
    StorageDeviceInfo(StorageDeviceInfo&&);
    StorageDeviceInfo(StorageDeviceInfo const&);
    StorageDeviceInfo operator =(StorageDeviceInfo&& );
    StorageDeviceInfo operator =(StorageDeviceInfo const& );
    ~StorageDeviceInfo(void);

    int vid(void) const noexcept;
    int pid(void) const noexcept;

    QString info(void) const noexcept;
    QString filePath(void) const noexcept;
    QList<devlib::Partition>  partitions(void) const;
    QList<devlib::Mountpoint> mountpoints(void) const;

    static QList<StorageDeviceInfo> availableDevices(void);

private:
    StorageDeviceInfo(int vid, int pid, QString const& devFilePath);
    std::unique_ptr<impl::StorageDeviceInfo_Private> _pimpl;
};

#endif // STORAGEDEVICE_H
