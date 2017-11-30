#ifndef CHECKSUMSUBTASK_H
#define CHECKSUMSUBTASK_H

#include <QObject>
#include <QFile>

#include "AbstractSubtask.h"
#include "shared/States.h"

class ChecksumSubtask : public AbstractSubtask
{
    Q_OBJECT
public:
    explicit ChecksumSubtask(QFile* image, QFile* device, QObject *parent = nullptr);
    virtual ~ChecksumSubtask(void) override;

    virtual void run(void) override;

signals:
    void progressChanged(uint value);
    void stateChanged(states::CheckingCorrectnessState state,
                      states::StateType                type = states::StateType::Info);

private:
    QFile*   _image;
    QFile*   _device;
};

#endif // CHECKSUMSUBTASK_H
