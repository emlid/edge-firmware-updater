#ifndef STORAGEDEVICEFILE_H
#define STORAGEDEVICEFILE_H

#include <QtCore>
#include <memory>

#include "StorageDeviceInfo.h"

namespace devlib {
    class StorageDeviceFile;
    class StorageDeviceInfo;

    namespace impl {
        class StorageDeviceFile_Private;
    }
}

class devlib::StorageDeviceFile : public QFile
{
    Q_OBJECT
public:
    StorageDeviceFile(devlib::StorageDeviceInfo const& info);
    ~StorageDeviceFile(void);

    virtual bool open(OpenMode mode) override;
    virtual void close(void) override;

private:
    std::unique_ptr<impl::StorageDeviceFile_Private> _pimpl;
};

#endif // STORAGEDEVICEFILE_H
