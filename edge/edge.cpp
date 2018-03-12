#include "edge.h"
#include "rpi.h"
#include "devlib.h"

#include "impl/EdgeManagerImpl.h"
#include "impl/EdgeDeviceImpl.h"

namespace local {
    static auto fileFactory(QString const& filename) {
        return std::make_unique<QFile>(filename);
    }

    static bool dirFactory(QString const& dirpath) {
        return QDir().mkpath(dirpath);
    }

    static auto edgeDeviceFactory(edge::EdgeConfig const& config,
                                  std::unique_ptr<devlib::IStorageDeviceInfo> info)
    {
        return std::make_unique<edge::EdgeDeviceImpl>
                (config, std::move(info), fileFactory, dirFactory);
    }

    static auto deviceList() {
        auto sdService = devlib::StorageDeviceService::instance();
        Q_ASSERT(sdService.get());
        return sdService->getAvailableStorageDevices();
    }
}

auto edge::makeEdgeManager(const edge::EdgeConfig &config)
    -> std::unique_ptr<edge::IEdgeManager>
{
    return std::make_unique<edge::EdgeManagerImpl>(
        config, usb::makeRpiBoot(config.vendorId(), {config.productId()}),
        local::deviceList, local::edgeDeviceFactory
    );
}
