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
    void setFilterParams(int vid, QList<int> pids);

    void runRpiBootStep(void) override final;

    void runDeviceScannerStep(void) override final;

    void runFlasherStep(QString firmwareFilename, bool checksumEnabled);

    void cancel(void) override final;

    void finish(void) override final;

signals:
    void execRpiBoot(void);

    void execDeviceScannerStep(void);

    void execFlasher(QString const& firmwareFilename, bool checksumEnabled);

    void setVidPid(int vid, QList<int> pids);

private slots:
    void _onRpiBootStateChanged(states::RpiBootState state, states::StateType type);
    void _onDeviceScannerStateChanged(states::DeviceScannerState state, states::StateType type);
    void _onFlasherStateChanged(states::FlasherState state, states::StateType type);

private:
    void _initConnections(FirmwareUpgrader*);

    QThread _thread;
};


#endif // REMOTEFLASHERWATCHER_H
