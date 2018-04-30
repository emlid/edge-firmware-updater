#ifndef EDGEMANAGERIMPL_H
#define EDGEMANAGERIMPL_H

#include "../EdgeManager.h"
#include "../EdgeConfig.h"
#include "logging.h"

#include "rpi.h"
#include "devlib.h"

namespace edge {
    class EdgeManagerImpl;
}

class edge::EdgeManagerImpl : public IEdgeManager
{
    Q_OBJECT
public:
    using DeviceListFunction_t = std::function<
        std::vector<std::unique_ptr<devlib::IStorageDeviceInfo>>(void)
    >;

    using EdgeDeviceFactory_t =  std::function<
        std::unique_ptr<edge::IEdgeDevice>(edge::EdgeConfig const&,
            std::unique_ptr<devlib::IStorageDeviceInfo>)
    >;


    EdgeManagerImpl(
            edge::EdgeConfig const& config,
            std::unique_ptr<usb::IRpiBoot> rpiboot,
            DeviceListFunction_t deviceListFunction,
            EdgeDeviceFactory_t edgeDeviceFactory,
            QObject* parent = nullptr
            )
        : IEdgeManager(parent),
          _config(config),
          _rpiboot(std::move(rpiboot)),
          _deviceListFunction(std::move(deviceListFunction)),
          _edgeDeviceFactory(std::move(edgeDeviceFactory))
    {
        Q_ASSERT(_rpiboot.get());

        QObject::connect(_rpiboot.get(), &usb::IRpiBoot::infoMessageReceived,
                         this,           &EdgeManagerImpl::infoMessageReceived);

        QObject::connect(_rpiboot.get(), &usb::IRpiBoot::warnMessageReceived,
                         this,           &EdgeManagerImpl::warnMessageReceived);

        QObject::connect(_rpiboot.get(), &usb::IRpiBoot::errorMessageReceived,
                         this,           &EdgeManagerImpl::errorMessageReceived);
    }

    virtual ~EdgeManagerImpl(void) = default;

private:
    virtual auto isEdgePlugged_core(void) const -> bool override;
    virtual auto isEdgeInitialized_core(void) const -> bool override;
    virtual auto initialize_core(void) -> std::unique_ptr<IEdgeDevice> override;

    bool _isEdgeAsMassStorage(std::unique_ptr<devlib::IStorageDeviceInfo> const&) const;

    edge::EdgeConfig _config;
    std::unique_ptr<usb::IRpiBoot> _rpiboot;
    DeviceListFunction_t _deviceListFunction;
    EdgeDeviceFactory_t  _edgeDeviceFactory;
};

#endif // EDGEMANAGERIMPL_H
