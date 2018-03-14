#ifndef EDGEDEVICEIMPL_H
#define EDGEDEVICEIMPL_H

#include "../EdgeDevice.h"
#include "../EdgeConfig.h"

#include "devlib.h"

namespace edge {
    class EdgeDeviceImpl;
}

class edge::EdgeDeviceImpl : public IEdgeDevice
{
public:
    using FileFactoryFunction_t =
        std::function<std::unique_ptr<QIODevice>(QString const&)>;

    using DirFactoryFunction_t =
        std::function<bool(QString const&)>;

    EdgeDeviceImpl(edge::EdgeConfig const& config,
                   std::unique_ptr<devlib::IStorageDeviceInfo>&& storageDeviceInfo,
                   FileFactoryFunction_t const& fileFactory,
                   DirFactoryFunction_t  const& dirFactory)
        : _config(config),
          _storageDeviceInfo(std::move(storageDeviceInfo)),
          _fileFactory(fileFactory),
          _dirFactory(dirFactory)
    { }

    virtual ~EdgeDeviceImpl(void) = default;

private:
    virtual auto isEdgeStillAvailable_core(void) const -> bool override;
    virtual auto firmwareVersion_core(void) const -> QString override;
    virtual auto asIODevice_core(void) const -> std::unique_ptr<QIODevice> override;
    virtual auto lock_core(void) -> bool override;

    edge::EdgeConfig _config;
    std::shared_ptr<devlib::IStorageDeviceInfo> _storageDeviceInfo;
    FileFactoryFunction_t _fileFactory;
    DirFactoryFunction_t  _dirFactory;
    std::list<
        std::unique_ptr<devlib::IMountpointLock>
    > _mntptLocks;
};

#endif // EDGEDEVICEIMPL_H
