#ifndef FIRMWAREUPDATESESSION_H
#define FIRMWAREUPDATESESSION_H

#include <QtCore>

#include "util.h"
#include "shared/shared.h"
#include "edge.h"
#include "devlib.h"

namespace updater {
    class FirmwareUpdateSession;
}

class updater::FirmwareUpdateSession : public QObject
{
    Q_OBJECT
public:
    FirmwareUpdateSession(edge::EdgeConfig config,
                          std::shared_ptr<util::IFlashingService> flashingService,
                          std::shared_ptr<util::ICRCService> crcService,
                          QObject* parent = nullptr)
        : QObject(parent),
          _sessionData(config)
    {
        auto cancellationCondition =
            [this] (void) { return _sessionData.cancellationPoint.isCancellationRequested(); };

        auto progressReporter =
            [this] (auto curr, auto total) { emit this->progressChanged(util::computePercent(curr, total)); };

        _flashDevice = [this, flashingService, cancellationCondition, progressReporter]
            (auto image, auto iodevice) {
                return flashingService->flash(
                     image, iodevice, cancellationCondition, progressReporter
                );
            };

        _calculateCrc = [this, crcService, cancellationCondition, progressReporter]
            (auto file, auto length) {
                return crcService->computeCRC(
                    file, length, cancellationCondition, progressReporter
                );
            };
    }

public slots:
    void initializeDevice(void);
    void flash(QString const& firmwareFilePath);
    void computeCRC(QString const& firmwareFilePath);

    void cancel(void) { _sessionData.cancellationPoint.cancel(); }

signals:
    void initializingFinished (updater::shared::OperationStatus status);
    void flashingFinished     (updater::shared::OperationStatus status);
    void crcFinished          (updater::shared::OperationStatus status);

    void logMessage(QString msg,
                    updater::shared::LogMessageType type);

    void progressChanged(int progress);
    void cancelled(void);
    void edgeFirmwareVersion(QString firmwareVersion);

private:
    auto _initializeDevice(void) -> shared::OperationStatus;
    auto _flash(QString const& firmwareFilePath) -> shared::OperationStatus;
    auto _computeCRC(QString const& firmwareFilePath) -> shared::OperationStatus;

    auto _mapRetStatusToOpStatus(
        util::IOService::RetStatus result,
        QString const& onCancel,   QString const& onSuccess,
        QString const& onReadFail, QString const& onWriteFail)
        -> shared::OperationStatus;

    void sendLogMessage(QString msg,
                        updater::shared::LogMessageType type = updater::shared::LogMessageType::Info)
    {
        qInfo() << msg; emit logMessage(msg, type);
    }

    struct SessionData_private {
        SessionData_private(edge::EdgeConfig conf)
            : config(conf) { }

        edge::EdgeConfig config;
        util::CancellationPoint cancellationPoint;
        std::unique_ptr<devlib::IStorageDeviceFile> edgeIODevice;
        std::unique_ptr<edge::IEdgeDevice> edgeDevice;
    } _sessionData;

    std::function<util::IOService::RetStatus(QIODevice*, QIODevice*)>        _flashDevice;
    std::function<std::pair<QByteArray, util::IOService::RetStatus>(QIODevice*, qint64)>  _calculateCrc;
};

#endif // FIRMWAREUPDATESESSION_H
