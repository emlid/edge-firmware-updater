#ifndef REMOTEFLASHERWATCHER_H
#define REMOTEFLASHERWATCHER_H


#include "Flasher.h"
#include "rep_FlasherWatcher_source.h"


class RemoteFlasherWatcher : public FlasherWatcherSimpleSource
{

    Q_OBJECT
public:
    explicit RemoteFlasherWatcher(QObject *parent = nullptr);

    virtual void start(void) override;

public slots:
    void setRpiBootState(FlasherState value);

    void setDeviceScannerState(FlasherState value);

    void setFirmwareUpgraderState(FlasherState value);

private slots:
    void onFlashStarted();

    void onFlashCompleted();

    void onProgressChanged(uint progress);

    void onFlashFailed(Flasher::FlashingStatus status);

};


#endif // REMOTEFLASHERWATCHER_H
