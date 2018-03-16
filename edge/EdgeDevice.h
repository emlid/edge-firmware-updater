#ifndef EDGEDEVICE_H
#define EDGEDEVICE_H

#include <memory>
#include <QIODevice>
#include "StorageDeviceFile.h"

namespace edge {
    class IEdgeDevice;
}

class edge::IEdgeDevice {
public:
    static auto undefinedVersion(void) { return QStringLiteral("Undefined"); }

    bool isEdgeStillAvailable(void) const { return isEdgeStillAvailable_core(); }
    auto firmwareVersion(void)      const { return firmwareVersion_core(); }
    auto asIODevice(void)           const { return asIODevice_core(); }
    virtual ~IEdgeDevice(void) = default;

private:
    virtual bool isEdgeStillAvailable_core(void) const = 0;
    virtual auto firmwareVersion_core(void) const -> QString = 0;
    virtual auto asIODevice_core(void) const -> std::unique_ptr<devlib::IStorageDeviceFile> = 0;
};

#endif // EDGEDEVICE_H
