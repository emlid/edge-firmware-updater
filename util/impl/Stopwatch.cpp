#include "Stopwatch.h"

using stopwatch = std::chrono::high_resolution_clock;

util::Stopwatch::Duration
    util::Stopwatch::elapsed(void)
{
    stop();

    auto workingTime = _finishTime - _startTime;
    auto chronoSeconds = std::chrono::
        duration_cast<std::chrono::seconds>(workingTime);

    auto duration = util::Stopwatch::Duration();

    duration.mins = chronoSeconds.count() / 60;
    duration.secs = chronoSeconds.count() - duration.mins * 60;

    return duration;
}
