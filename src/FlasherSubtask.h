#ifndef FLASHERSUBTASK_H
#define FLASHERSUBTASK_H

#include "AbstractSubtask.h"
#include "shared/States.h"

#include <QtCore>

class FlasherSubtask : public AbstractSubtask
{
    Q_OBJECT
public:
    explicit FlasherSubtask(QFile& src, QFile& dest, QObject *parent = nullptr);
    virtual ~FlasherSubtask(void) override;

    virtual void run(void) override;

signals:
    void progressChanged(uint value);
    void stateChanged(states::FlasherState state,
                      states::StateType    type = states::StateType::Info);

private:
    QFile& _src;
    QFile& _dest;
};

#endif // FLASHERSUBTASK_H
