#ifndef SHARED_H
#define SHARED_H

#include <cstdint>

namespace updater{
    namespace shared {
        enum OperationStatus : std::int8_t {
            Succeed = 1, Cancelled = 0, Failed
        };

        enum LogMessageType : std::int8_t {
            Info = 0, Warning, Error
        };
    }
}

#endif // SHARED_H
