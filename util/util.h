#ifndef UTIL_H
#define UTIL_H

#include <chrono>
#include <QThread>

#include "FlashingService.h"
#include "CRCService.h"
#include "CancellationPoint.h"
#include "impl/Stopwatch.h"

using namespace std::chrono_literals;

namespace util {
    using microsecs = std::chrono::microseconds;
    constexpr auto defaultPeriod(void)   { return microsecs{10}; }
    constexpr auto defaultPollTime(void) { return microsecs{5000}; }

    auto makeFlashingService(void) -> std::unique_ptr<IFlashingService>;
    auto makeCRCService(void)      -> std::unique_ptr<ICRCService>;

    template<typename T> T poll(
            std::function<T(void)> callback,
            microsecs const& pollTime = defaultPollTime(),
            microsecs const& period   = defaultPeriod())
    {
        auto curPollTime = microsecs{0};
        do {
            auto result = callback();
            if (result) {
                return result;
            }

            QThread::msleep(period.count());
            curPollTime += period;
        } while (curPollTime < pollTime);

        return {};
    }

    constexpr int computePercent(qint64 curr, qint64 total) {
        return (curr * 100) / total;
    }
}

#endif // UTIL_H
