#ifndef RPIBOOTSUBTASK_H
#define RPIBOOTSUBTASK_H

#include "AbstractSubtask.h"
#include "shared/States.h"

class RpiBootSubtask : public AbstractSubtask
{
    Q_OBJECT
public:
    explicit RpiBootSubtask(int vid, QList<int> const& pids, QObject *parent = nullptr);
    virtual ~RpiBootSubtask(void) override;

    virtual void run(void) override;

signals:
    void stateChanged(states::RpiBootState state,
                      states::StateType    type = states::StateType::Info);

private:
    int        _vid;
    QList<int> _pids;
};

#endif // RPIBOOTSUBTASK_H
