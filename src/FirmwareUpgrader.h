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
    static const QString RPIBOOT_SUBSYSTEM_NAME;
    static const QString DEVICESCANNER_SUBSYSTEM_NAME;
    static const QString FLASHER_SUBSYSTEM_NAME;

    enum State {
        Started = 0,
        Finished,
        UnexpectedError,
        DeviceFound,
        DeviceNotFound,
        OpenImageFailed,
        OpenDeviceFailed,
        ImageReadingFailed,
        DeviceWritingFailed,
        CheckingCorrectness
    };

    explicit FirmwareUpgrader(QString const& firmwareFilename, QObject *parent = nullptr);

    bool runRpiBootStep(int vid, QList<int> const& pids);

    bool runDeviceScannerStep(int vid, QList<int> const& pids, QVector<std::shared_ptr<StorageDevice>>* physicalDrives);

    bool runFlashingDeviceStep(QVector<std::shared_ptr<StorageDevice>> const& physicalDrives);


signals:
    void subsystemStateChanged(QString const& subsystem, uint value);

    void flashingProgressChanged(uint value);

public slots:
    void start(void);

private slots:
    void _onFlashStarted();

    void _onFlashCompleted();

    void _onProgressChanged(uint progress);

    void _onFlashFailed(Flasher::FlashingStatus status);

private:
    bool _checkCorrectness(QFile& image, QFile& device);

    void _setCurrentState(QString const& subsystem, State value);

    void _runAllSteps(void);

    QString _firmwareFilename;
};

#endif // FIRMWAREUPGRADER_H
