#ifndef CANCELLABLE_H
#define CANCELLABLE_H

#include <QObject>
#include <functional>

class Cancellable
{
public:
    using CancelCondition_t = std::function<bool(void)>;

    explicit Cancellable()
        : _cancelCondition([](){return false;}), _wasCancelled(false)
    { }

    void setCancelCondition(CancelCondition_t condition) {
        _cancelCondition = condition;
    }

    CancelCondition_t cancelCondition(void) const {
        return _cancelCondition;
    }

    bool wasCancelled(void) const {
        return _wasCancelled;
    }

protected:
    bool _cancelRequested(void) {
        return _cancelCondition();
    }

    void _informAboutCancel(void) {
        _wasCancelled = true;
    }

private:
    CancelCondition_t _cancelCondition;
    bool              _wasCancelled;
};

#endif // CANCELLABLE_H
