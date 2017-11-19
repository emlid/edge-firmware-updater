#ifndef REMOTEFLASHERWATCHER_H
#define REMOTEFLASHERWATCHER_H

#include <QtCore>

#include "rep_FirmwareUpgraderWatcher_source.h"
#include "FirmwareUpgrader.h"


class FirmwareUpgraderWatcher : public FirmwareUpgraderWatcherSimpleSource
{
    Q_OBJECT
public:
    explicit FirmwareUpgraderWatcher(QObject *parent = nullptr);

public slots:
    void setVidPid(int vid, QList<int> pids);

    void runRpiBootStep       (void) override;
    void runDeviceScannerStep (void) override;
    void runFlasherStep       (QString firmwareFilename, bool checksumEnabled) override;

    void finish(void) override;

signals:
    void _setVidPid(int vid, QList<int> pids);

    void _stop                  (void);
    void _execRpiBoot           (void);
    void _execDeviceScannerStep (void);
    void _execFlasher           (QString const& firmwareFilename, bool checksumEnabled);

private slots:
    void _onRpiBootStateChanged       (states::RpiBootState state, states::StateType type);
    void _onDeviceScannerStateChanged (states::DeviceScannerState state, states::StateType type);
    void _onFlasherStateChanged       (states::FlasherState state, states::StateType type);

private:
    void _initConnections(FirmwareUpgrader*);

    QThread _thread;
};


#endif // REMOTEFLASHERWATCHER_H
