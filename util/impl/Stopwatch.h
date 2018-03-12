#ifndef STOPWATCH_H
#define STOPWATCH_H

#include <QString>

#include <inttypes.h>
#include <utility>
#include <chrono>

namespace util {
    class Stopwatch;
}

class util::Stopwatch
{
public:
    struct Duration {
        int64_t mins;
        int64_t secs;

        QString asQString(void) const {
            auto leadSymbol = QChar('0');
            auto leadCount  = 2;
            auto base       = 10;

            return QString("%1:%2")
                .arg(mins, leadCount, base, leadSymbol)
                .arg(secs, leadCount, base, leadSymbol);
        }
    };

    void start(void) { _startTime = clk::now(); }
    void stop(void)  { _finishTime = clk::now(); }

    Duration elapsed(void);

private:
    using clk = std::chrono::high_resolution_clock;
    std::chrono::time_point<clk>  _startTime, _finishTime;
};


#endif // STOPWATCH_H
