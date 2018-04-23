#ifndef CLIENT_H
#define CLIENT_H

#include <memory>

#include "UpdaterConnection.h"
#include "shared.h"

namespace client {
    std::unique_ptr<UpdaterConnection> makeConnection(void);
}

#endif // CLIENT_H
