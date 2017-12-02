#ifndef REMOTEFLASHERWATCHER_H
#define REMOTEFLASHERWATCHER_H

#include <QtCore>
#include <memory>

#include "rep_FirmwareUpgraderWatcher_source.h"
#include "SubtaskManager.h"
#include "shared/States.h"

class StorageDevice;

class FirmwareUpgraderWatcher : public FirmwareUpgraderWatcherSimpleSource
{
    Q_OBJECT
public:
    explicit FirmwareUpgraderWatcher(QObject *parent = nullptr);

public slots:
    void setVidPid(int vid, QList<int> pids);

    void runRpiBootStep             (void) override;
    void runDeviceScannerStep       (void) override;
    void runFlasherStep             (QString firmwareFilename) override;
    void runCheckingCorrectnessStep (void) override;

    void finish(void) override;
    void cancel(void) override;

signals:
    void _cancel(void);

private slots:
    void _exit(void);
    void _onNoActiveTasks(void);

    void _onRpiBootStateChanged       (states::RpiBootState             state, states::StateType type);
    void _onDeviceScannerStateChanged (states::DeviceScannerState       state, states::StateType type);
    void _onFlasherStateChanged       (states::FlasherState             state, states::StateType type);
    void _onChecksumCalcStateChanged  (states::CheckingCorrectnessState state, states::StateType type);

private:
    int        _vid;
    QList<int> _pids;

    std::shared_ptr<QFile> _image;
    std::shared_ptr<QFile> _device;

    SubtaskManager _taskManager;
    QVector<std::shared_ptr<StorageDevice>> _devices;
};


#endif // REMOTEFLASHERWATCHER_H
