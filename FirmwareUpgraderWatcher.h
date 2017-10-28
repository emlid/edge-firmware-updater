#ifndef REMOTEFLASHERWATCHER_H
#define REMOTEFLASHERWATCHER_H

#include <QtCore>

#include "rep_FirmwareUpgraderWatcher_source.h"


class FirmwareUpgraderWatcher : public FirmwareUpgraderWatcherSimpleSource
{
    Q_OBJECT

public:
    explicit FirmwareUpgraderWatcher(QObject *parent = nullptr);

    virtual void start(void) override;

private:
    QThread _thread;

};


#endif // REMOTEFLASHERWATCHER_H
