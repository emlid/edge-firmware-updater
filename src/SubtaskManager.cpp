#include <QtCore>
#include <QtConcurrent/QtConcurrent>
#include "SubtaskManager.h"


SubtaskManager::SubtaskManager(QObject *parent)
    : QObject(parent), _subtask(nullptr)
{
    QThreadPool::globalInstance()->reserveThread();
    QObject::connect(&_watcher, &QFutureWatcher<void>::finished,
                     this, &SubtaskManager::noActiveSubtasks);
}


bool SubtaskManager::run(std::unique_ptr<AbstractSubtask>&& subtask)
{
    Q_ASSERT(subtask != nullptr);

    if (_watcher.isRunning()) {
        qWarning("Task Manager is busy");
        return false;
    }

    QObject::connect(this, &SubtaskManager::_stopTask,
                     subtask.get(), &AbstractSubtask::cancel);

    _subtask = std::move(subtask);
    auto future = QtConcurrent::run([this] (void) { _subtask->run(); });
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



