#include "FirmwareUpgraderWatcher.h"
#include "FirmwareUpgrader.h"


FirmwareUpgraderWatcher::FirmwareUpgraderWatcher(QObject *parent)
    : FirmwareUpgraderWatcherSimpleSource(parent)
{ }


void FirmwareUpgraderWatcher::start(QString firmwareFilename, bool checksumEnabled)
{
    auto imageFileInfo = QFileInfo(firmwareFilename);

    qInfo() << "Firmware image filename: " << firmwareFilename;

    if (!(imageFileInfo.exists() && imageFileInfo.isReadable() && imageFileInfo.isFile())) {
        qCritical() << "incorrect image file.";
        emit subsystemStateChanged(QString("FirmwareUpgrader"),
                                   static_cast<uint>(FirmwareUpgrader::State::OpenImageFailed));
        return;
    }

    auto fwUpgrader = new FirmwareUpgrader(firmwareFilename, checksumEnabled);
    fwUpgrader->moveToThread(&_thread);

    QObject::connect(&_thread, &QThread::started,  fwUpgrader, &FirmwareUpgrader::start);
    QObject::connect(&_thread, &QThread::finished, fwUpgrader, &FirmwareUpgrader::deleteLater);

    QObject::connect(fwUpgrader, &FirmwareUpgrader::finished, this, &FirmwareUpgraderWatcher::finished);
    QObject::connect(fwUpgrader, &FirmwareUpgrader::finished, this, &FirmwareUpgraderWatcher::cancel);

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


void FirmwareUpgraderWatcher::cancel(void)
{
    if (_thread.isRunning()) {
        _thread.quit();
        _thread.wait();
    }

    emit finished();

    std::exit(0);
}
