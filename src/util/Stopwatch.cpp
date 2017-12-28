#include "Stopwatch.h"

using namespace util;
using stopwatch = std::chrono::high_resolution_clock;


Stopwatch::Stopwatch(void) { }


void Stopwatch::start(void) {
    _startTime = stopwatch::now();
}


void Stopwatch::stop(void) {
    _finishTime = stopwatch::now();
}


Stopwatch::Duration Stopwatch::elapsed(void) {
    stop();

    auto workingTime = _finishTime - _startTime;
    auto chronoSeconds = std::chrono::
        duration_cast<std::chrono::seconds>(workingTime);

    auto duration = util::Stopwatch::Duration();

    duration.mins = chronoSeconds.count() / 60;
    duration.secs = chronoSeconds.count() - duration.mins * 60;

    return duration;
}
