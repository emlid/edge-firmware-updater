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
    explicit AbstractSubtask(QString subtaskName = "Unknown", QObject *parent = nullptr)
        : QObject(parent), _subtaskName(subtaskName), _needToStop(false) { }

    virtual ~AbstractSubtask(void) { }

signals:
    void finished(bool status = true);

public slots:
    void stop(void) {
        _needToStop = true;
    }

protected:
    QString _subtaskMsg(QString const& msg) {
        return _subtaskName + ": " + msg;
    }

    bool _stopRequested(void) const {
        return _needToStop;
    }

    QString _subtaskName;

private:
    volatile bool _needToStop;
};

#endif // ABSTRACTSUBTASK_H
