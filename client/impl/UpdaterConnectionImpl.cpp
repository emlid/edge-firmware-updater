#include "UpdaterConnectionImpl.h"
#include "rep_EdgeFirmwareUpdaterIPC_replica.h"
#include "UpdaterImpl.h"


using Base = client::UpdaterConnection;
using ProcessFinishedSignal_t = void(client::UpdaterProcess::*)(int, client::UpdaterProcess::ExitStatus);


// Cast to finished signal (See Qt doc)
constexpr auto finishedSignal(void)
    -> ProcessFinishedSignal_t
{
    return &client::UpdaterProcess::finished;
}


client::UpdaterConnectionImpl::UpdaterConnectionImpl(
        client::UpdaterConnectionImpl::ProcessFactory_t const& procFactory,
        Config const& config, QObject* parent
)
    : Base(parent),
      _config(config),
      _makeProcess(procFactory)
{
    _disconnectTimer.setInterval(_config.disconnectTimeout);
    _disconnectTimer.setSingleShot(true);

    if (_config.enableHeartbeat) {
        _heartbeatTimer.setInterval(_config.heartbeatPeriod);
        _heartbeatTimer.setSingleShot(true);

        QObject::connect(&_heartbeatTimer, &QTimer::timeout,
                         this, &UpdaterConnectionImpl::_sendHeartbeat);
    }
}


client::UpdaterConnectionImpl::~UpdaterConnectionImpl(void)
{
    _disconnectTimer.blockSignals(true);
    _heartbeatTimer.blockSignals(true);
    _heartbeatTimer.stop();

    if (Base::currentState() == State::Disconnecting) {
        qWarning() << "Deleting object in 'Disconnecting' state.";
        while (_proc->state() == UpdaterProcess::Running) {
            QCoreApplication::processEvents();
        }
    }

    // If replica is initialized that means the process
    // run with admin privileges. The only proper way to kill him
    // it's finish its via replica (not QProcess::kill)
    if (_replica && _replica->isReplicaValid()) {
        QObject::disconnect(_processStateConnection);
        QObject::disconnect(_replicaStateConnection);
        _replica->finish();

        while (_proc->state() == UpdaterProcess::Running) {
            QCoreApplication::processEvents();
        }
    }
    // Replica is not initialized. That means the process
    // runs with graphical sudo (trying to get admin privileges from user).
    // We can kill him via QProcess API
    else if (_proc && _proc->isOpen()) {
        _proc->blockSignals(true);
        _proc->kill();
        _proc->waitForFinished();
    }
}


void client::UpdaterConnectionImpl::_establish(QString const& updaterExeName,
                                               QStringList const& updaterArgs)
{
    _proc = _makeProcess();

    if (_config.verbose) {
        QObject::connect(_proc.get(), &UpdaterProcess::readyRead,
            [this] (void) { qDebug() << " -- Updater process output: " << _proc->readAll(); }
        );
    }

    QObject::connect(_proc.get(), &UpdaterProcess::started,
                     this,        &UpdaterConnectionImpl::_connectToUpdaterNode);

    _processStateConnection =
            QObject::connect(_proc.get(), finishedSignal(),
                             this, &UpdaterConnectionImpl::_handleProcessFinished);

    _proc->startAsAdmin(updaterExeName, updaterArgs);
}


void client::UpdaterConnectionImpl::
    _handleProcessFinished(int exitCode, UpdaterProcess::ExitStatus status)
{
    Base::_setErrorString(_proc->readAllStandardError());

    auto state = status == UpdaterProcess::ExitStatus::CrashExit ?
                State::Aborted : State::Disconneted;

    if (exitCode) {
        qDebug() << detailedErrorDescription();
        qDebug() << "Process errored with exit code " << exitCode;
    }

    Base::_changeState(state);
    _heartbeatTimer.stop();
}


void client::UpdaterConnectionImpl::_connectToUpdaterNode(void)
{
    using Replica = EdgeFirmwareUpdaterIPCReplica;

    auto success = _node.connectToNode(_config.updaterReplicaNodeName);

    if (!success) {
        Base::_setErrorString("Can not connect to updater node");
        Base::_changeState(State::Errored);
        return;
    }

    _replica.reset(_node.acquire<Replica>());

    QObject::connect(_replica.get(), &Replica::initialized,
        [this] (void) {
            _heartbeatTimer.start();

            Base::_updater.reset(new UpdaterImpl(_replica));
            Base::_changeState(State::Established);

            // If replica state was changed to not valid then this means
            // the firmware updater process was forcefully or unexpectedly fails
            auto replicaStateListener =
                [this] (Replica::State cur, Replica::State old) {
                    Q_UNUSED(cur);
                    if (old == Replica::State::Valid) {
                        Base::_setErrorString("Firmware updater unexpectedly finished.");
                        Base::_changeState(State::Aborted);
                    }
                };

            QObject::disconnect(_processStateConnection);
            _replicaStateConnection = QObject::connect(_replica.get(),
                                                       &Replica::stateChanged,
                                                       replicaStateListener);
        }
    );
}


void client::UpdaterConnectionImpl::_sendHeartbeat(void)
{
    _heartbeatTimer.start();
    emit _replica->heartbeat();
}


void client::UpdaterConnectionImpl::_sever(void)
{
    QObject::disconnect(_replicaStateConnection);

    QObject::connect(_proc.get(), finishedSignal(),
                     this, &UpdaterConnectionImpl::_handleProcessFinished);

    auto lambdaCtx = std::shared_ptr<QObject>(new QObject);

    QObject::connect(&_disconnectTimer, &QTimer::timeout, lambdaCtx.get(),
        [this, lambdaCtx] (void) mutable {
            if (Base::currentState() != State::Disconneted) {
                qWarning() << "Can not perform soft disconnect. Kill process forcefully.";
                _proc->kill();
            }
            lambdaCtx.reset();
        }
    );

    Base::_updater->finish();
    _replica.reset();
    _disconnectTimer.start();
}
