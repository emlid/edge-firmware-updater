#ifndef EDGEFIRMWAREUPDATER_H
#define EDGEFIRMWAREUPDATER_H

#include <QtCore>
#include <memory>

#include "FirmwareUpdateSession.h"
#include "rep_EdgeFirmwareUpdaterIPC_source.h"
#include "edge.h"

class EdgeFirmwareUpdater : public EdgeFirmwareUpdaterIPCSimpleSource
{
    Q_OBJECT
public:
    explicit EdgeFirmwareUpdater(QObject *parent = nullptr)
        : EdgeFirmwareUpdaterIPCSimpleSource(parent),
          _config(0xa5c, 0x2764, 0x0001, "issue.txt", "temp_mnt", "boot"),
          _flashingService(util::makeFlashingService()),
          _crcService(util::makeCRCService())
    { }

public slots:
    void openSession(void) override {
        qInfo() << "open sessiong message received";
        if (_updateSession) {
            emit logMessage("firmware update session not closed",
                            int(updater::shared::Warning));
            return;
        }

        _updateSession = std::make_unique<updater::FirmwareUpdateSession>
                (_config, _flashingService, _crcService);
        _updateSession->moveToThread(&_sessionThread);

        _connectToSession();
        _sessionThread.start();
    }

    void closeSession(void) override {
        if (_updateSession) {
            _sessionThread.quit();
            _sessionThread.wait();
            _updateSession.reset();
        } else {
            qWarning() << "Sessiong already closed";
        }
    }

    void initializeEdgeDevice (void) override {
        Q_ASSERT(_updateSession);
        qInfo() << "initialize edge device message received.";
        emit _initializeEdgeDevice();
    }

    void flash(QString firmwareFilename) override {
        Q_ASSERT(_updateSession);
        qInfo() << "flash edge device message received";
        _firmwareFilename = firmwareFilename;
        emit _flash(firmwareFilename);
    }

    void checkOnCorrectness(void) override {
        Q_ASSERT(_updateSession);
        qInfo() << "compute crc message received";
        emit _checkOnCorrectness(_firmwareFilename);
    }

    void finish(void) override {
        std::exit(EXIT_SUCCESS);
    }

    void cancel(void) override {
        Q_ASSERT(_updateSession);
        emit _cancel();
    }

signals:
    void _cancel(void);
    void _initializeEdgeDevice(void);
    void _flash(QString firmwareFilename);
    void _checkOnCorrectness(QString firmwareFilename);

private:
    void _connectToSession() {
        using EdgeFwUpdater = EdgeFirmwareUpdater;
        using UpdaterSession = updater::FirmwareUpdateSession;

        QObject::connect(this, &EdgeFwUpdater::_initializeEdgeDevice,
            _updateSession.get(), &UpdaterSession::initializeDevice);

        QObject::connect(this, &EdgeFwUpdater::_flash,
            _updateSession.get(), &UpdaterSession::flash);

        QObject::connect(this, &EdgeFwUpdater::_checkOnCorrectness,
            _updateSession.get(), &UpdaterSession::computeCRC);

        QObject::connect(this, &EdgeFwUpdater::_cancel,
            _updateSession.get(), &UpdaterSession::cancel, Qt::DirectConnection);

        QObject::connect(_updateSession.get(), &UpdaterSession::logMessage,
                         this, &EdgeFwUpdater::logMessage);

        QObject::connect(_updateSession.get(), &UpdaterSession::edgeFirmwareVersion,
                         this, &EdgeFwUpdater::firmwareVersion);

        QObject::connect(_updateSession.get(), &UpdaterSession::cancelled,
                         this, &EdgeFwUpdater::cancelled);

        QObject::connect(_updateSession.get(), &UpdaterSession::crcFinished,
                         this, &EdgeFwUpdater::checkOnCorrectnessFinished);

        QObject::connect(_updateSession.get(), &UpdaterSession::flashingFinished,
                         this, &EdgeFwUpdater::flashingFinished);

        QObject::connect(_updateSession.get(), &UpdaterSession::initializingFinished,
                         this, &EdgeFwUpdater::initializingFinished);

        QObject::connect(_updateSession.get(), &UpdaterSession::progressChanged,
                         this, &EdgeFwUpdater::progressChanged);

    }

    QString          _firmwareFilename;
    QThread          _sessionThread;
    edge::EdgeConfig _config;
    std::shared_ptr<util::IFlashingService> _flashingService;
    std::shared_ptr<util::ICRCService>      _crcService;
    std::unique_ptr<updater::FirmwareUpdateSession> _updateSession;
};

#endif // EDGEFIRMWAREUPDATER_H
