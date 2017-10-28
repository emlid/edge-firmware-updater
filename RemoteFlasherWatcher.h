#ifndef REMOTEFLASHERWATCHER_H
#define REMOTEFLASHERWATCHER_H

#include <memory>

#include "Flasher.h"
#include "rep_FlasherWatcher_source.h"


class StorageDevice;


class RemoteFlasherWatcher : public FlasherWatcherSource
{

    Q_OBJECT
public:
    explicit RemoteFlasherWatcher(QObject *parent = nullptr);

    virtual void start(void) override;

    bool runRpiBootStep(int vid, QList<int> const& pids);

    bool runDeviceScannerStep(int vid, QList<int> const& pids, QVector<std::shared_ptr<StorageDevice>>* physicalDrives);

    bool runFlashingDeviceStep(QVector<std::shared_ptr<StorageDevice>> const& physicalDrives);

public slots:
    void setRpiBootState(FlasherState value);

    void setDeviceScannerState(FlasherState value);

    void setFirmwareUpgraderState(FlasherState value);

    void runAllSteps(void);

private slots:
    void _onFlashStarted();

    void _onFlashCompleted();

    void _onProgressChanged(uint progress);

    void _onFlashFailed(Flasher::FlashingStatus status);

};


#endif // REMOTEFLASHERWATCHER_H
