#include "FirmwareUpgraderWatcher.h"
#include "FirmwareUpgrader.h"


FirmwareUpgraderWatcher::FirmwareUpgraderWatcher(QObject *parent)
    : FirmwareUpgraderWatcherSimpleSource(parent)
{ }


void FirmwareUpgraderWatcher::start(void)
{
    auto fwUpgrader = new FirmwareUpgrader();
    fwUpgrader->moveToThread(&_thread);

    connect(&_thread, &QThread::started,  fwUpgrader, &FirmwareUpgrader::start);
    connect(&_thread, &QThread::finished, fwUpgrader, &FirmwareUpgrader::deleteLater);

    auto stateChangedConnection = std::shared_ptr<QMetaObject::Connection>();
    *stateChangedConnection = connect(fwUpgrader, &FirmwareUpgrader::subsystemStateChanged,
            [this] (QString const& subsystem, FirmwareUpgrader::State state) {
                emit subsystemStateChanged(subsystem, static_cast<uint>(state));
            }
    );

    auto progressChangedConnection = std::shared_ptr<QMetaObject::Connection>();
    *progressChangedConnection = connect(fwUpgrader, &FirmwareUpgrader::flashingProgressChanged,
            [this] (uint progress) {
                emit flasherProgressChanged(progress);
            }
    );

    connect(this, &FirmwareUpgraderWatcher::destroyed,
        [stateChangedConnection, progressChangedConnection] () {
            QObject::disconnect(*stateChangedConnection);
            QObject::disconnect(*progressChangedConnection);
        }
    );

    _thread.start();
}
