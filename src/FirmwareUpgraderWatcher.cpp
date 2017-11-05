#include "FirmwareUpgraderWatcher.h"
#include "FirmwareUpgrader.h"


FirmwareUpgraderWatcher::FirmwareUpgraderWatcher(QObject *parent)
    : FirmwareUpgraderWatcherSimpleSource(parent)
{ }


void FirmwareUpgraderWatcher::start(QString firmwareFilename)
{
    auto imageFileInfo = QFileInfo(firmwareFilename);

    qInfo() << "Firmware image filename: " << firmwareFilename;

    if (!(imageFileInfo.exists() && imageFileInfo.isReadable() && imageFileInfo.isFile())) {
        qCritical() << "incorrect image file.";
        emit subsystemStateChanged(QString("FirmwareUpgrader"),
                                   static_cast<uint>(FirmwareUpgrader::State::OpenImageFailed));
        return;
    }

    auto fwUpgrader = new FirmwareUpgrader(firmwareFilename);
    fwUpgrader->moveToThread(&_thread);

    connect(&_thread, &QThread::started,  fwUpgrader, &FirmwareUpgrader::start);
    connect(&_thread, &QThread::finished, fwUpgrader, &FirmwareUpgrader::deleteLater);

    QObject::connect(fwUpgrader, &FirmwareUpgrader::subsystemStateChanged, this,
                     &FirmwareUpgraderWatcher::_onSubsystemStateChanged);

    QObject::connect(fwUpgrader, &FirmwareUpgrader::flashingProgressChanged,
                     this, &FirmwareUpgraderWatcher::flasherProgressChanged);

    _thread.start();
}


void FirmwareUpgraderWatcher::_onSubsystemStateChanged(QString subsystem, uint state)
{
    emit subsystemStateChanged(subsystem, static_cast<uint>(state));
}
