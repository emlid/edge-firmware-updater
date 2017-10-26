#ifndef REMOTEFLASHERWATCHER_H
#define REMOTEFLASHERWATCHER_H


#include "rep_FlasherWatcher_source.h"


class RemoteFlasherWatcher : public FlasherWatcherSimpleSource
{
    Q_OBJECT
public:
    explicit RemoteFlasherWatcher(QObject *parent = nullptr);

signals:

public slots:
};

#endif // REMOTEFLASHERWATCHER_H
