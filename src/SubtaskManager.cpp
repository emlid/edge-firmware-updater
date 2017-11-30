#include <QtCore>
#include <QtConcurrent/QtConcurrent>
#include "SubtaskManager.h"


SubtaskManager::SubtaskManager(QObject *parent)
    : QObject(parent)
{
    QThreadPool::globalInstance()->reserveThread();
    QObject::connect(&_watcher, &QFutureWatcher<void>::finished,
                     this, &SubtaskManager::noActiveSubtasks);
}


bool SubtaskManager::run(AbstractSubtask* subtask)
{
    Q_ASSERT(subtask != nullptr);

    if (_watcher.isRunning()) {
        qCritical("Task Manager is busy");
        return false;
    }

    subtask->setAutoDelete(true);

    QObject::connect(this, &SubtaskManager::_stopTask,
                     subtask, &AbstractSubtask::stop);

    auto future = QtConcurrent::run([subtask] (void) { subtask->run(); });
    _watcher.setFuture(future);

    return future.isRunning();
}


bool SubtaskManager::hasActiveSubtasks(void) const
{
    return !_watcher.isFinished();
}


void SubtaskManager::stopTask(void)
{
    emit _stopTask();
}



