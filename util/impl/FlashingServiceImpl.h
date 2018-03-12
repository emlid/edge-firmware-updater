#ifndef FLASHINGSERVICEIMPL_H
#define FLASHINGSERVICEIMPL_H

#include "../FlashingService.h"

namespace util {
    class FlashingServiceImpl;
}

class util::FlashingServiceImpl : public util::IFlashingService
{
private:
    virtual RetStatus flash_core(
            QIODevice* src, QIODevice* dest,
            CancellationCondition_t cancellationCondition,
            ProgressReporter_t progressReporter,
            int blockSize) override;
};

#endif // FLASHINGSERVICEIMPL_H
