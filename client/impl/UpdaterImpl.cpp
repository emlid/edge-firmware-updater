#include "UpdaterImpl.h"
#include "rep_EdgeFirmwareUpdaterIPC_replica.h"
#include "shared.h"

namespace Shared = updater::shared;


client::UpdaterImpl::UpdaterImpl(std::shared_ptr<EdgeFirmwareUpdaterIPCReplica> updaterReplica,
                                 QObject* parent)
    : client::Updater(parent),
      _sessionOpened(false),
      _updaterReplica(updaterReplica)
{
    if (_updaterReplica && _updaterReplica->isReplicaValid()) {
        _attachToUpdaterReplica();
        _makeProxies();
        _changeState(State::Idle);
    }
}


client::UpdaterImpl::~UpdaterImpl(void)
{
    if (isValid()) {
        _updaterReplica->finish();
    }
}


bool client::UpdaterImpl::_initializeDevice(void)
{
    // Open new update session
    _closeUpdateSession();
    _openUpdateSession();

    _updaterReplica->initializeEdgeDevice();
    _changeState(State::Initializing);
    return true;
}


bool client::UpdaterImpl::_flash(QString const& firmwarePath, bool checksumEnabled)
{
    if (!_sessionOpened) {
        qWarning() << "Trying to flash without opened session";
        return false;
    }

    using Replica = EdgeFirmwareUpdaterIPCReplica;

    auto lastAction = &Replica::flashingFinished;
    auto lambdaCtx = std::shared_ptr<QObject>(nullptr);

    if (checksumEnabled) {
        // Make a lambda context (used for delete connection, after handling
        // was performed), Qt 5.2 feature
        lastAction = &Replica::checkOnCorrectnessFinished;
        lambdaCtx.reset(new QObject);

        auto onFlashingFinished =
            [this, lambdaCtx] (int stat) mutable {
                lambdaCtx.reset();
                if (stat == Shared::OperationStatus::Succeed) {
                    _updaterReplica->checkOnCorrectness();
                    _changeState(State::ComputingChecksum);

                } else if (stat == Shared::OperationStatus::Failed) {
                    emit deviceFlashed(false);
                    _changeState(State::Idle);
                }
            };

        QObject::connect(_updaterReplica.get(), &Replica::flashingFinished,
                         lambdaCtx.get(), onFlashingFinished);
    }

    lambdaCtx.reset(new QObject);
    auto onUpdatingFinished =
        [this, lambdaCtx] (int stat) mutable {
            lambdaCtx.reset();
            if (stat == Shared::OperationStatus::Succeed) {
                emit deviceFlashed(true);
            } else if (stat == Shared::OperationStatus::Failed) {
                emit deviceFlashed(false);
            }
            _changeState(State::Idle);
        };

    QObject::connect(_updaterReplica.get(), lastAction,
                     lambdaCtx.get(), onUpdatingFinished);

    _updaterReplica->flash(firmwarePath);
    _changeState(State::Flashing);
    return true;
}


bool client::UpdaterImpl::_cancel(void)
{
    _updaterReplica->cancel();
    return true;
}


bool client::UpdaterImpl::_finish(void)
{
    _updaterReplica->finish();
    return true;
}


void client::UpdaterImpl::_handleReplicaLogMessage(QString msg, int type)
{
    auto msgType = static_cast<Shared::LogMessageType>(type);
    switch (msgType) {
        case Shared::LogMessageType::Info:    emit infoMessageReceived(msg);  break;
        case Shared::LogMessageType::Warning: emit warnMessageReceived(msg);  break;
        case Shared::LogMessageType::Error:   emit errorMessageReceived(msg); break;
    }
}


void client::UpdaterImpl::_attachToUpdaterReplica(void)
{
    using Replica = EdgeFirmwareUpdaterIPCReplica;

    auto updaterPtr = _updaterReplica.get();

    connect(updaterPtr, &Replica::stateChanged,
        [this] (Replica::State current, Replica::State old) {
            Q_UNUSED(current);
            if (old == Replica::State::Valid) {
                _changeState(State::Invalid);
            }
        }
    );

    connect(updaterPtr, &Replica::progressChanged, this, &UpdaterImpl::updateProgressChanged);
    connect(updaterPtr, &Replica::cancelled,       this, &UpdaterImpl::cancelled);
    connect(updaterPtr, &Replica::firmwareVersion, this, &UpdaterImpl::firmwareVersion);
    connect(updaterPtr, &Replica::logMessage,      this, &UpdaterImpl::_handleReplicaLogMessage);
}


void client::UpdaterImpl::_makeProxies(void)
{
    connect(this, &client::UpdaterImpl::deviceFlashed,
        [this] (bool stat) {
            Q_UNUSED(stat);
            _closeUpdateSession();
        }
    );
}


void client::UpdaterImpl::_openUpdateSession(void)
{
    _updaterReplica->openSession();
    _sessionOpened = true;
}


void client::UpdaterImpl::_closeUpdateSession(void)
{
    _updaterReplica->closeSession();
    _sessionOpened = false;
}
