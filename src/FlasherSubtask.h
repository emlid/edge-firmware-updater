#ifndef FLASHERSUBTASK_H
#define FLASHERSUBTASK_H

#include "AbstractSubtask.h"
#include "shared/States.h"

#include <QtCore>
#include <memory>

class FlasherSubtask : public AbstractSubtask
{
    Q_OBJECT
public:
    explicit FlasherSubtask(std::shared_ptr<QFile> src,
                            std::shared_ptr<QFile> dest,
                            QObject *parent = nullptr);

    virtual ~FlasherSubtask(void) override;

    virtual void run(void) override;

signals:
    void progressChanged(uint value);
    void stateChanged(states::FlasherState state,
                      states::StateType    type = states::StateType::Info);

private:
    std::shared_ptr<QFile> _src;
    std::shared_ptr<QFile> _dest;
};

#endif // FLASHERSUBTASK_H
