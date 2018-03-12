#ifndef CRCSERVICEIMPL_H
#define CRCSERVICEIMPL_H

#include "../CRCService.h"

namespace util {
    class CRCServiceImpl;
}

class util::CRCServiceImpl : public ICRCService
{
private:
    virtual std::pair<QByteArray, RetStatus> computeCRC_core(
            QIODevice* source, qint64 length,
            CancellationCondition_t cancellationCondition,
            ProgressReporter_t progressReporter,
            qint64 ioBlockSize) override;
};

#endif // DSFG_H
