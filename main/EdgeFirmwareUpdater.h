#ifndef EDGEFIRMWAREUPDATER_H
#define EDGEFIRMWAREUPDATER_H

#include <QtCore>
#include <memory>

#include "utilities.h"
#include "FirmwareUpdateSession.h"
#include "rep_EdgeFirmwareUpdaterIPC_source.h"
#include "edge.h"


class EdgeFirmwareUpdater : public EdgeFirmwareUpdaterIPCSimpleSource
{
    using Updater = EdgeFirmwareUpdater;

    Q_OBJECT
public:
    explicit EdgeFirmwareUpdater(QObject *parent = nullptr)
        : EdgeFirmwareUpdaterIPCSimpleSource(parent),
          _config(0xa5c, 0x2764, 0x0001, 1000, "issue.txt", "temp_mnt", "boot"),
          _flashingService(util::makeFlashingService()),
          _crcService(util::makeCRCService())
    {
        auto const heartbeatLatency = 1000;

        _heartbeatTimer.setInterval(_config.heartbeatPeriod() + heartbeatLatency);
        _heartbeatTimer.setSingleShot(true);

        QObject::connect(&_heartbeatTimer, &QTimer::timeout,
            [this] (void) {
                qCWarning(logg::basic()) << "Timeout: updater didn't receive heartbeat over"
                           << _heartbeatTimer.interval() << "ms";
                finish();
            }
        );

        _heartbeatTimer.start();
    }

public slots:
    void openSession(void) override {
        qCInfo(logg::basic()) << "OpenSession: command received";
        if (_updateSession) {
            emit logMessage("Updater session is not closed",
                            int(updater::shared::Warning));
            return;
        }

        _updateSession = std::make_unique<updater::FirmwareUpdateSession>
                (_config, _flashingService, _crcService);
        _updateSession->moveToThread(&_sessionThread);

        _connectToSession();
        _sessionThread.start();
        qCInfo(logg::basic()) << "OpenSession: performed";
    }

    void closeSession(void) override {
        qCInfo(logg::basic()) << "CloseSession: command received";
        if (_updateSession) {
            _sessionThread.quit();
            _sessionThread.wait();
            _updateSession.reset();
            qCInfo(logg::basic()) << "CloseSession: performed";
        } else {
            qCWarning(logg::basic()) << "CloseSession: Session already closed";
        }
    }

    void initializeEdgeDevice(void) override {
        checkSessionBeforePerform(&Updater::_initializeEdgeDevice);
    }

    void flash(QString firmwareFilename) override {
        _firmwareFilename = firmwareFilename;
        checkSessionBeforePerform(&Updater::_flash, firmwareFilename);
    }

    void checkOnCorrectness(void) override {
        checkSessionBeforePerform(&Updater::_checkOnCorrectness, _firmwareFilename);
    }

    void finish(void) override {
        std::exit(EXIT_SUCCESS);
    }

    void cancel(void) override {
        checkSessionBeforePerform(&Updater::_cancel);
    }

    void heartbeat(void) override {
        _heartbeatTimer.start();
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
                         this, &Updater::cancelled);

        QObject::connect(_updateSession.get(), &UpdaterSession::crcFinished,
                         this, &EdgeFwUpdater::checkOnCorrectnessFinished);

        QObject::connect(_updateSession.get(), &UpdaterSession::flashingFinished,
                         this, &EdgeFwUpdater::flashingFinished);

        QObject::connect(_updateSession.get(), &UpdaterSession::initializingFinished,
                         this, &EdgeFwUpdater::initializingFinished);

        QObject::connect(_updateSession.get(), &UpdaterSession::progressChanged,
                         this, &EdgeFwUpdater::progressChanged);
    }

    template<class Signal_t, class ...Args>
    void checkSessionBeforePerform(Signal_t signalPtr, Args&& ...args)
    {
        auto signal = std::mem_fn(signalPtr);

        if (_updateSession) {
            emit signal(this, std::forward<Args>(args)...);
        } else {
            qCCritical(logg::basic()) << "Can not perform. Session is not opened";
        }
    }

    QTimer           _heartbeatTimer;
    QString          _firmwareFilename;
    QThread          _sessionThread;
    edge::EdgeConfig _config;
    std::shared_ptr<util::IFlashingService> _flashingService;
    std::shared_ptr<util::ICRCService>      _crcService;
    std::unique_ptr<updater::FirmwareUpdateSession> _updateSession;
};

#endif // EDGEFIRMWAREUPDATER_H
