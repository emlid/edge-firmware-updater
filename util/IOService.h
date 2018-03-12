#ifndef IOSERVICE_H
#define IOSERVICE_H

#include <QIODevice>
#include <functional>

namespace util {
    class IOService;
}

class util::IOService
{
public:
    using ProgressReporter_t = std::function<void(qint64,qint64)>;
    using CancellationCondition_t = std::function<bool(void)>;

    static auto defaultProgressReporter(void) { return [] (auto, auto) {}; }
    static auto defaultCancellationCondition(void) { return [] () { return false; }; }
    static auto defaultBlockSize(void) { return 65536LL; }

    enum class RetStatus {
        Success, ReadFail, WriteFail, Cancelled
    };

    virtual ~IOService(void) = default;
};

#endif // IOSERVICE_H
