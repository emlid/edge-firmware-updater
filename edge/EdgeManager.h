#ifndef EDGEMANAGER_H
#define EDGEMANAGER_H

#include <memory>
#include "EdgeDevice.h"

namespace edge {
    class IEdgeManager;
}

class edge::IEdgeManager {
public:
    bool isEdgePlugged(void) const { return isEdgePlugged_core(); }
    bool isEdgeInitialized(void) const { return isEdgeInitialized_core(); }
    auto initialize(void) { return initialize_core(); }
    virtual ~IEdgeManager(void) = default;

private:
    virtual bool isEdgePlugged_core(void) const = 0;
    virtual bool isEdgeInitialized_core(void) const = 0;
    virtual auto initialize_core(void) -> std::unique_ptr<edge::IEdgeDevice> = 0;
};

#endif // EDGEMANAGER_H
