#ifndef ABSTRACTSUBTASK_H
#define ABSTRACTSUBTASK_H

#include <QObject>
#include <QRunnable>

/*
 * An abstract class for subtasks which can be runned in reusable thread and can be cancelled.
*/

class AbstractSubtask : public QObject, public QRunnable
{
    Q_OBJECT
public:
    enum ExitStatus {
        Succeed = 1,
        Cancelled = 0,
        Failed = -1
    };

    enum LogMessageType {
        Info = 0,
        Warning,
        Error
    };

    explicit AbstractSubtask(QString subtaskName = "Unknown", QObject *parent = nullptr)
        : QObject(parent), _subtaskName(subtaskName), _needToCancel(false) { }

    virtual ~AbstractSubtask(void) { }

    QString subtaskName(void) const { return subtaskName(); }

signals:
    void finished(AbstractSubtask::ExitStatus status = Succeed);
    void logMessage(QString msg, AbstractSubtask::LogMessageType type);

public slots:
    void cancel(void) {
        _needToCancel = true;
    }

protected:
    void sendLogMessage(QString const& msg, AbstractSubtask::LogMessageType type = Info) {
        emit logMessage(_subtaskName + ": " + msg, type);
    }

    bool stopRequested(void) const {
        return _needToCancel;
    }

    QString _subtaskName;

private:
    volatile bool _needToCancel;
};

#endif // ABSTRACTSUBTASK_H
