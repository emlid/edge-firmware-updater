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

    virtual void start(QString firmwareFilename, bool checksumEnabled) override;
    virtual void cancel(void) override;

private:
    QThread _thread;

private slots:
    void _onSubsystemStateChanged(QString subsystem, uint state);
};


#endif // REMOTEFLASHERWATCHER_H
