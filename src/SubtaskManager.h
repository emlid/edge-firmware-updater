#ifndef SUBTASKMANAGER_H
#define SUBTASKMANAGER_H

#include <QObject>
#include <QFutureWatcher>
#include <memory>

#include "AbstractSubtask.h"

class SubtaskManager : public QObject
{
    Q_OBJECT
public:
    explicit SubtaskManager(QObject *parent = nullptr);

    bool run               (std::unique_ptr<AbstractSubtask>&& subtask);
    bool hasActiveSubtasks (void) const;

    AbstractSubtask* currentSubtask(void) {
        return _subtask.get();
    }

signals:
    void noActiveSubtasks (void);
    void _stopTask        (void);

public slots:
    void stopTask(void);

private:
    std::unique_ptr<AbstractSubtask> _subtask;
    QFutureWatcher<void>             _watcher;
};

#endif // SUBTASKMANAGER_H
