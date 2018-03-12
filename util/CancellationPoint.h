#ifndef CANCELLATIONPOINT_H
#define CANCELLATIONPOINT_H

#include <list>
#include <functional>

namespace util {
    class CancellationPoint;
}

class util::CancellationPoint
{
public:
    using Action_t = std::function<void(void)>;

    CancellationPoint(void)
        : _isCancellationRequested(false)
    { }

    bool isCancellationRequested(void) {
        return _isCancellationRequested;
    }

    void cancel(void) {
        _isCancellationRequested = true;
        for (auto const& action : _actions) { action(); }
    }

    void onCancellationRequested(Action_t action);

private:
    volatile bool       _isCancellationRequested;
    std::list<Action_t> _actions;
};

#endif // CANCELLATIONPOINT_H
