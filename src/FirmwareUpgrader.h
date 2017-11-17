#ifndef FIRMWAREUPGRADER_H
#define FIRMWAREUPGRADER_H

#include <QtCore>
#include <memory>

#include "Flasher.h"
#include "shared/States.h"


class StorageDevice;


class FirmwareUpgrader : public QObject
{
    Q_OBJECT

public:
    explicit FirmwareUpgrader(QObject *parent = nullptr);

public slots:
    void setVidPid(int vid, QList<int> const& _pids);

    bool runRpiBootStep(void);

    bool runDeviceScannerStep(void);

    bool runFlashingDeviceStep(
            QString const& firmwareFilename,
            bool checksumEnabled);

signals:
    void finished(void);

    void deviceMountpoints(QStringList mntpts);

    void flasherStateChanged(states::FlasherState const& state,
        states::StateType const& type = states::StateType::Info);

    void deviceScannerStateChanged(states::DeviceScannerState const& state,
        states::StateType const& type = states::StateType::Info);

    void rpibootStateChanged(states::RpiBootState const& state,
        states::StateType const& type = states::StateType::Info);

    void flashingProgressChanged(uint value);

private slots:
    void _onFlashStarted();

    void _onProgressChanged(uint progress);

    void _onFlashFailed(Flasher::FlashingStatus status);

private:
    bool _checkCorrectness(QFile& image, QFile& device);

    QVector<std::shared_ptr<StorageDevice>> _physicalDrives;
    int _vid;
    QList<int> _pids;
};

#endif // FIRMWAREUPGRADER_H
