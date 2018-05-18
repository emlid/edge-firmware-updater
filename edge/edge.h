#ifndef EDGE_H
#define EDGE_H

#include <QString>
#include <QIODevice>

#include "EdgeConfig.h"
#include "EdgeDevice.h"
#include "EdgeManager.h"

namespace edge {
    auto makeEdgeManager(edge::EdgeConfig const& config)
        -> std::unique_ptr<edge::IEdgeManager>;

    auto makeDefaultEdgeConfig(void)
        -> EdgeConfig;
}

#endif // EDGE_H
