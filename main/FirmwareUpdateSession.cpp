#include "FirmwareUpdateSession.h"

#include <utilities.h>
#include <edge.h>

#include <QFile>

using MsgType  = updater::shared::LogMessageType;
using OpStatus = updater::shared::OperationStatus;


namespace Tags {
    using OnReadFailedMsg  = QString;
    using OnWriteFailedMsg = QString;
    using OnSuccessMsg     = QString;
    using OnCancelMsg      = QString;
}


void updater::FirmwareUpdateSession::initializeDevice(void)
{
    sendLogMessage("Initializing...");

    auto edgeManager = edge::makeEdgeManager(_sessionData.config);

    QObject::connect(edgeManager.get(), &edge::IEdgeManager::infoMessageReceived,
                     [this] (QString const& msg) { sendLogMessage(msg, MsgType::Info); });

    QObject::connect(edgeManager.get(), &edge::IEdgeManager::warnMessageReceived,
                     [this] (QString const& msg) { sendLogMessage(msg, MsgType::Warning); });

    QObject::connect(edgeManager.get(), &edge::IEdgeManager::errorMessageReceived,
                     [this] (QString const& msg) { sendLogMessage(msg, MsgType::Error); });

    Q_ASSERT(edgeManager.get());

    if (!(edgeManager->isEdgePlugged() || edgeManager->isEdgeInitialized())) {
        sendLogMessage("Edge is not plugged", MsgType::Error);
        emit initializingFinished(OpStatus::Failed);
        return;
    }

    _sessionData.edgeDevice = edgeManager->initialize();

    if (!_sessionData.edgeDevice) {
        sendLogMessage("initialization failed.", MsgType::Error);
        emit initializingFinished(OpStatus::Failed);
    } else {
        emit edgeFirmwareVersion(_sessionData.edgeDevice->firmwareVersion());
        sendLogMessage("Edge successfully initialized");
        emit initializingFinished(OpStatus::Succeed);
    }
}


auto updater::FirmwareUpdateSession::
    _mapRetStatusToOpStatus(util::IOService::RetStatus result,
                            QString const& onCancel,   QString const& onSuccess,
                            QString const& onReadFail, QString const& onWriteFail)
    -> shared::OperationStatus
{
    Q_UNUSED(onCancel); Q_UNUSED(onSuccess);
    using Result = util::IOService::RetStatus;

    switch (result) {
        case Result::Cancelled: { return OpStatus::Cancelled; }
        case Result::Success:   { return OpStatus::Succeed; }

        case Result::ReadFail: {
            sendLogMessage(onReadFail, MsgType::Error);
            return OpStatus::Failed;
        }

        case Result::WriteFail: {
            sendLogMessage(onWriteFail, MsgType::Error);
            return OpStatus::Failed;
        }

        default: {
            sendLogMessage("Flasher: Undefined msg", MsgType::Error);
            return OpStatus::Failed;
        }
    }
}


auto updater::FirmwareUpdateSession::_flash(QString const& firmwareFilePath)
    -> shared::OperationStatus
{
    sendLogMessage("Flasher: flashing...");
    QFile imageFile(firmwareFilePath);
    _sessionData.edgeIODevice = _sessionData.edgeDevice->asIODevice();
    auto& iodevice = _sessionData.edgeIODevice;

    imageFile.open(QIODevice::ReadOnly);
    iodevice->open(QIODevice::ReadWrite);

    auto stopwatch = util::Stopwatch();
    stopwatch.start();
    auto result = _flashDevice(&imageFile, iodevice.get());
    sendLogMessage("Flasher: elapsed time " + stopwatch.elapsed().asQString());

    iodevice->sync();

    auto taskName = "Flasher";
    return _mapRetStatusToOpStatus(result,
        Tags::OnCancelMsg{": cancelled"}.prepend(taskName),
        Tags::OnSuccessMsg{": successfully finished"}.prepend(taskName),
        Tags::OnReadFailedMsg{": can not read from image"}.prepend(taskName),
        Tags::OnWriteFailedMsg{": can not write to the device"}.prepend(taskName)
    );
}


void updater::FirmwareUpdateSession::flash(const QString &firmwareFilePath)
{
    auto result = _flash(firmwareFilePath);
    if (result == OpStatus::Cancelled) {
        emit cancelled();
    }
    emit flashingFinished(result);
}


auto updater::FirmwareUpdateSession::_computeCRC(QString const& firmwareFilePath)
    -> shared::OperationStatus
{

    auto mapCrcResultToOpResult = [this] (util::IOService::RetStatus result, QString const& file) {
        auto taskName = "Checksum";
        return _mapRetStatusToOpStatus(result,
            Tags::OnCancelMsg{": cancelled"}.prepend(taskName),
            Tags::OnSuccessMsg{": successfully finished"}.prepend(taskName),
            Tags::OnReadFailedMsg{": can not read from "}.append(file).prepend(taskName),
            Tags::OnWriteFailedMsg{": unexpected write error"}.prepend(taskName)
        );
    };

    QFile imageFile(firmwareFilePath);
    if (!imageFile.open(QIODevice::ReadOnly)) {
        sendLogMessage("Checksum: Can not open image file", MsgType::Error);
        return OpStatus::Failed;
    }

    auto stopwatch = util::Stopwatch();
    stopwatch.start();

    sendLogMessage("Checksum: compute image crc");
    auto imgResult = _calculateCrc(&imageFile, imageFile.size());
    auto opStatus = mapCrcResultToOpResult(imgResult.second, "image");
    if (opStatus != OpStatus::Succeed) {
        return opStatus;
    }

    auto& iodevice = _sessionData.edgeIODevice;
    iodevice->seek(0);

    sendLogMessage("Checksum: compute device crc");
    auto devResult = _calculateCrc(iodevice.get(), imageFile.size());
    opStatus = mapCrcResultToOpResult(devResult.second, "device");
    if (opStatus != OpStatus::Succeed) {
        return opStatus;
    }

    sendLogMessage("Checksum: elapsed time " + stopwatch.elapsed().asQString());

    if (imgResult.first == devResult.first) {
        sendLogMessage("Image successfully wrote");
        return OpStatus::Succeed;
    } else {
        sendLogMessage("Image incorrectly wrote", MsgType::Error);
        return OpStatus::Failed;
    }
}


void updater::FirmwareUpdateSession::
    computeCRC(QString const& firmwareFilePath)
{
    auto result = _computeCRC(firmwareFilePath);
    if (result == OpStatus::Cancelled) {
        emit cancelled();
    }
    emit crcFinished(result);
}
