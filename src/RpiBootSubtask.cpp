#include "RpiBootSubtask.h"
#include "devapi/rpiboot/rpiboot.h"


RpiBootSubtask::RpiBootSubtask(int vid, QList<int> const& pids, QObject *parent)
    : AbstractSubtask("RpiBootSubtask", parent), _vid(vid), _pids(pids)
{ }


RpiBootSubtask::~RpiBootSubtask(void)
{
    qDebug() << _subtaskMsg("finished");
}


void RpiBootSubtask::run(void)
{
    using RBState = states::RpiBootState;
    using StType  = states::StateType;

    if (_stopRequested()) {
        emit finished();
        return;
    }

    RpiBoot rpiboot(_vid, _pids);

    emit stateChanged(RBState::RpiBootStarted);
    qInfo() << _subtaskMsg("Started...");

    auto successful = rpiboot.rpiDevicesCount() > 0
                   && rpiboot.bootAsMassStorage() == 0;

    if (!successful) {
        qCritical() << _subtaskMsg("Rpiboot failed");
        emit stateChanged(RBState::RpiBootFailed, StType::Error);
        emit finished(false);
        return;
    }

    emit stateChanged(RBState::RpiBootFinished);
    emit finished();
}
