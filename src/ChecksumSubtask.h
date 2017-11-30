#ifndef CHECKSUMSUBTASK_H
#define CHECKSUMSUBTASK_H

#include <QObject>
#include <QFile>
#include <memory>

#include "AbstractSubtask.h"
#include "shared/States.h"

class ChecksumSubtask : public AbstractSubtask
{
    Q_OBJECT
public:
    explicit ChecksumSubtask(std::shared_ptr<QFile> image,
                             std::shared_ptr<QFile> device,
                             QObject *parent = nullptr);

    virtual ~ChecksumSubtask(void) override;

    virtual void run(void) override;

signals:
    void progressChanged(uint value);
    void stateChanged(states::CheckingCorrectnessState state,
                      states::StateType                type = states::StateType::Info);

private:
    std::shared_ptr<QFile>   _image;
    std::shared_ptr<QFile>   _device;
};

#endif // CHECKSUMSUBTASK_H
