#ifndef CRCSERVICE_H
#define CRCSERVICE_H

#include <QIODevice>
#include "IOService.h"

namespace util {
    class ICRCService;
}

class util::ICRCService : public IOService
{
public:
    auto computeCRC(
            QIODevice* source, qint64 length,
            CancellationCondition_t cancellationCondition = defaultCancellationCondition(),
            ProgressReporter_t progressReporter = defaultProgressReporter(),
            qint64 ioBlockSize = defaultBlockSize())
    {
        Q_ASSERT(source);
        Q_ASSERT(length > 0);
        Q_ASSERT(source->isReadable());
        Q_ASSERT(ioBlockSize > 0);

        return computeCRC_core(source, length, cancellationCondition, progressReporter, ioBlockSize);
    }

    virtual ~ICRCService(void) = default;

private:
    virtual std::pair<QByteArray, RetStatus> computeCRC_core(
            QIODevice* source, qint64 length,
            CancellationCondition_t cancellationCondition,
            ProgressReporter_t progressReporter,
            qint64 ioBlockSize) = 0;
};


#endif // CRCSERVICE_H
