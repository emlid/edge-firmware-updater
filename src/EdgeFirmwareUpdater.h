#ifndef REMOTEFLASHERWATCHER_H
#define REMOTEFLASHERWATCHER_H

#include <QtCore>
#include <memory>

#include "rep_EdgeFirmwareUpdaterIPC_source.h"
#include "SubtaskManager.h"
#include "edge.h"
#include "FlasherDataOwner.h"

class EdgeFirmwareUpdater : public EdgeFirmwareUpdaterIPCSimpleSource
{
    Q_OBJECT
public:
    explicit EdgeFirmwareUpdater(QObject *parent = nullptr);

public slots:
    void initializeEdgeDevice (void) override;
    void flash                (QString firmwareFilename) override;
    void checkOnCorrectness   (void) override;

    void finish(void) override;
    void cancel(void) override;

signals:
    void _cancel(void);

private slots:
    void _exit(void);
    void _onNoActiveTasks(void);
    void _onLogMessageReceived(QString msg, AbstractSubtask::LogMessageType type);

private:
    FlasherDataOwner             _flasherData;
    SubtaskManager               _taskManager;
    util::Optional<edge::Device> _edgeDevice;
};


#endif // REMOTEFLASHERWATCHER_H
