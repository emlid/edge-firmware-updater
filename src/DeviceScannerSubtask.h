#ifndef DEVICESCANNERSUBTASK_H
#define DEVICESCANNERSUBTASK_H

#include <memory>

#include "AbstractSubtask.h"
#include "shared/States.h"
#include "devapi/StorageDevice.h"


class DeviceScannerSubtask : public AbstractSubtask
{
    Q_OBJECT
public:
    explicit DeviceScannerSubtask(int vid, QList<int> const& pids, QObject *parent = nullptr);
    virtual ~DeviceScannerSubtask(void) override;

    virtual void run(void) override;

    QVector<std::shared_ptr<StorageDevice>> result(void) const {
        return _physicalDrives;
    }

signals:
    void stateChanged(states::DeviceScannerState state,
                      states::StateType          type = states::StateType::Info);

private:
    int        _vid;
    QList<int> _pids;

    QVector<std::shared_ptr<StorageDevice>> _physicalDrives;
};

#endif // DEVICESCANNERSUBTASK_H
