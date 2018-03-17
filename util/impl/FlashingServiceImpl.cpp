#include "FlashingServiceImpl.h"
#include "../util.h"

#include <QtCore>

util::IFlashingService::RetStatus
    util::FlashingServiceImpl::flash_core(QIODevice* src, QIODevice* dest,
                                          CancellationCondition_t cancellationCondition,
                                          ProgressReporter_t progressReport,
                                          int blockSize)
{
    auto srcSize  = src->size();
    auto totalWrote = 0LL;

    auto prevProgress = 0LL;

    while (!src->atEnd()) {
        if (cancellationCondition()) {
            return RetStatus::Cancelled;
        }

        auto buffer = src->read(blockSize);

        if (buffer.isEmpty()) {
            return RetStatus::ReadFail;
        }

        auto readed = buffer.size();
        auto wrote  = dest->write(buffer);

        if (wrote != readed) {
            return RetStatus::WriteFail;
        }

        totalWrote += wrote;
        auto currentProgress = util::computePercent(totalWrote, srcSize);
        if (prevProgress != currentProgress) {
            progressReport(totalWrote, srcSize);
            prevProgress = currentProgress;
        }
    }

    return RetStatus::Success;
}
