#ifndef FLASHINGSERVICE_H
#define FLASHINGSERVICE_H

#include <QObject>
#include <QIODevice>
#include <functional>
#include <memory>
#include "IOService.h"

namespace util {
    class IFlashingService;
}

class util::IFlashingService : public IOService
{
public:
    virtual ~IFlashingService(void) = default;

    auto flash(QIODevice* src, QIODevice* dest,
               CancellationCondition_t cancellationCondition = defaultCancellationCondition(),
               ProgressReporter_t progressReport = defaultProgressReporter(),
               int blockSize = defaultBlockSize())
    { return flash_core(src, dest, cancellationCondition, progressReport, blockSize); }

private:
    virtual RetStatus flash_core(QIODevice* src, QIODevice* dest,
                                 CancellationCondition_t cancellationCondition,
                                 ProgressReporter_t progressReport,
                                 int blockSize) = 0;

};


#endif // FLASHINGSERVICE_H
