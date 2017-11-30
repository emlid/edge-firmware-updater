#ifndef SUBTASKMANAGER_H
#define SUBTASKMANAGER_H

#include <QObject>
#include <QFutureWatcher>

#include "AbstractSubtask.h"

class SubtaskManager : public QObject
{
    Q_OBJECT
public:
    explicit SubtaskManager(QObject *parent = nullptr);

    bool run               (AbstractSubtask* subtask);
    bool hasActiveSubtasks (void) const;

signals:
    void noActiveSubtasks(void);
    void _stopTask(void);

public slots:
    void stopTask(void);

private:
    QFutureWatcher<void> _watcher;
};

#endif // SUBTASKMANAGER_H
