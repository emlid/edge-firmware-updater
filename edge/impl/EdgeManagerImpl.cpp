#include "EdgeManagerImpl.h"
#include "util.h"


auto edge::EdgeManagerImpl::
    _isEdgeAsMassStorage(std::unique_ptr<devlib::IStorageDeviceInfo> const& storageDevice)
    const -> bool
{
    Q_ASSERT(storageDevice.get());
    return storageDevice->vid() == _config.vendorId()
        && storageDevice->pid() == _config.productIdAsMassStorage();
}


auto edge::EdgeManagerImpl::isEdgePlugged_core() const
    -> bool
{
    return usb::countOfDevicesWith(_config.vendorId(), {_config.productId()}) > 0;
}


auto edge::EdgeManagerImpl::isEdgeInitialized_core(void)
    const -> bool
{
    auto storageDevices = _deviceListFunction();
    return std::any_of(storageDevices.cbegin(),
                       storageDevices.cend(),
                       [this] (auto const& dev) { return this->_isEdgeAsMassStorage(dev); });

}


static auto logStorageDeviceInfo(devlib::IStorageDeviceInfo const* deviceInfo)
{
    qCInfo(edge::edgelog()) << "Short info: "
                      << QString("vid: %1").arg(deviceInfo->vid())
                      << QString("pid: %1").arg(deviceInfo->pid())
                      << QString("filePath: %1").arg(deviceInfo->filePath());

    auto mountpoints = deviceInfo->mountpoints();
    qCInfo(edge::edgelog()) << QString("Mountpoints count: %1").arg(mountpoints.size());

    auto partitions = deviceInfo->partitions();
    qCInfo(edge::edgelog()) << QString("Partitions count: %1").arg(partitions.size());
}


auto edge::EdgeManagerImpl::initialize_core(void)
    -> std::unique_ptr<IEdgeDevice>
{
    if (!isEdgeInitialized()) {
        if (_rpiboot->bootAsMassStorage() == -1) {
            qCWarning(edgelog()) << "Rpiboot failed";
            return {};
        }
    }

    using EdgeDevice = std::unique_ptr<edge::IEdgeDevice>;
    auto device = util::poll<EdgeDevice>(
        [this] (void) -> EdgeDevice {
            auto storageDevices = _deviceListFunction();
            auto edgePtr        = std::find_if(storageDevices.begin(),
                                               storageDevices.end(),
                                               [this] (auto const& dev) { return this->_isEdgeAsMassStorage(dev); });

            if (edgePtr == storageDevices.end()) {
                return nullptr;
            } else {
                ::logStorageDeviceInfo((*edgePtr).get());

                auto device = _edgeDeviceFactory(_config, std::move(*edgePtr));
                storageDevices.erase(edgePtr);
                return device;
            }
        }
    );

    if (!device) {
        qCWarning(edgelog()) << "Timeout: edge as storage device not found";
    }

    return device;
}
