#ifndef FIRMWAREUPGRADER_H
#define FIRMWAREUPGRADER_H

#include <QtCore>
#include <memory>

#include "Flasher.h"
#include "rep_FirmwareUpgraderWatcher_source.h"


class StorageDevice;


class FirmwareUpgrader : public QObject
{
    Q_OBJECT

public:
    enum class State {
        Started = 0,
        Finished,
        UnexpectedError,
        DeviceFound,
        DeviceNotFound,
        OpenImageFailed,
        OpenDeviceFailed,
        ImageReadingFailed,
        DeviceWritingFailed
    };

    explicit FirmwareUpgrader(QString const& firmwareFilename, QObject *parent = nullptr);

    bool runRpiBootStep(int vid, QList<int> const& pids);

    bool runDeviceScannerStep(int vid, QList<int> const& pids, QVector<std::shared_ptr<StorageDevice>>* physicalDrives);

    bool runFlashingDeviceStep(QVector<std::shared_ptr<StorageDevice>> const& physicalDrives);

signals:
    void subsystemStateChanged(QString const& subsystem, State value);

    void flashingProgressChanged(uint value);

public slots:
    void start(void);

private slots:
    void _onFlashStarted();

    void _onFlashCompleted();

    void _onProgressChanged(uint progress);

    void _onFlashFailed(Flasher::FlashingStatus status);

private:
    void _setCurrentState(QString const& subsystem, State value);

    void _runAllSteps(void);

    QString _firmwareFilename;
};

#endif // FIRMWAREUPGRADER_H
