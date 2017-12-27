#ifndef EDGE_H
#define EDGE_H

#include "devlib/devlib.h"
#include "util/util.h"

namespace edge {
    class Device;
    using Initializer = std::function<util::Optional<Device>(void)>;

    bool available(void);
    util::Optional<Initializer> connect(void);

    constexpr int vid(void) { return 0x0a5c; }
    constexpr int pid(void) { return 0x2764; }
    constexpr int pidAsMassStorage(void) { return 0x0001; }

    bool isEdgeAsMassStorage(devlib::StorageDeviceInfo const& device);

    namespace impl {
        util::Optional<edge::Device> initialize(void);
    }
}


class edge::Device
{
    friend util::Optional<Device> impl::initialize(void);
public:
    Device(Device&&) noexcept;
    Device(Device const&);
    Device& operator =(Device&&) noexcept;
    Device& operator =(Device const&);
    ~Device(void) {}

    bool    stillAvailable(void)  const;
    QString firmwareVersion(void);

    devlib::StorageDeviceInfo& asStorageDevice(void) {
        return *(_deviceInfo.get());
    }

private:
    explicit Device(devlib::StorageDeviceInfo const& device);
    explicit Device(void);

    util::Optional<devlib::Partition>  _bootPartition(void);
    util::Optional<devlib::Mountpoint> _bootPartMountpoint(
            devlib::Partition& bootPart);

    static QString _bootMountpointPath();
    static QString _fwVersionFilename();

    std::unique_ptr<devlib::StorageDeviceInfo> _deviceInfo;
};

#endif // EMLIDEDGEDEVICE_H
