#ifndef DEVICESCANNERSUBTASK_H
#define DEVICESCANNERSUBTASK_H

#include <memory>

#include "AbstractSubtask.h"
#include "edge.h"


class DeviceInitializerSubtask : public AbstractSubtask
{
    Q_OBJECT
public:
    explicit DeviceInitializerSubtask(QObject* parent = nullptr);
    virtual ~DeviceInitializerSubtask(void) override;

    virtual void run(void) override;

    static QString name(void) {
        return QString("Initializer");
    }

signals:
    void deviceAvailable(edge::Device edgeDevice);
    void edgeVersion(QString version);

private:
    int        _vid;
    QList<int> _pids;
};

#endif // DEVICESCANNERSUBTASK_H
