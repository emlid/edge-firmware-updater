#ifndef CANCELLABLE_H
#define CANCELLABLE_H

#include <QObject>
#include <functional>

class Cancellable
{
public:
    using CancelCodition_t = std::function<bool(void)>;

    explicit Cancellable()
        : _cancelCondition([](){return false;}), _wasCancelled(false)
    { }

    void setCancelCondition(CancelCodition_t condition) {
        _cancelCondition = condition;
    }

    CancelCodition_t cancelCodition(void) const {
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
    CancelCodition_t _cancelCondition;
    bool             _wasCancelled;
};

#endif // CANCELLABLE_H
