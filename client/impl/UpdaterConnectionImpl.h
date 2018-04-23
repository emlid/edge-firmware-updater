#ifndef UPDATERCONNECTIONIMPL_H
#define UPDATERCONNECTIONIMPL_H

#include <QtCore>
#include <QtRemoteObjects>

#include <memory>

#include "UpdaterConnection.h"
#include "UpdaterProcess.h"
#include "../../main/shared/shared.h"

class EdgeFirmwareUpdaterIPCReplica;

class UpdaterConnectionImpl : public UpdaterConnection
{
    Q_OBJECT
public:
    using ProcessFactory_t = std::function<
        std::unique_ptr<UpdaterProcess>(void)
    >;

    struct Config {
        int disconnectTimeout;
        int connectTimeout;
        int heartbeatPeriod;
        bool enableHeartbeat;
        QString updaterReplicaNodeName;
        bool verbose;

        static Config defaultConfig(void) {
            return {
                1500, 1500,
                updater::shared::properties.heartbeatPeriod,
                updater::shared::properties.heartbeatEnabled,
                updater::shared::properties.updaterReplicaNodeName,
                true
            };
        }
    };

    UpdaterConnectionImpl(ProcessFactory_t const& procFactory,
                          Config const& config = Config::defaultConfig(),
                          QObject* parent = nullptr);
    ~UpdaterConnectionImpl(void) override;

private slots:
    void _handleProcessFinished(int exitCode, UpdaterProcess::ExitStatus);
    void _connectToUpdaterNode(void);
    void _sendHeartbeat(void);

private:
    virtual void _establish(QString const& updaterExePath) override;
    virtual void _sever(void) override;


    Config     _config;

    QRemoteObjectNode _node;
    QTimer _disconnectTimer;
    QTimer _heartbeatTimer;

    std::shared_ptr<EdgeFirmwareUpdaterIPCReplica> _replica;
    std::unique_ptr<UpdaterProcess> _proc;

    QMetaObject::Connection _processStateConnection;
    QMetaObject::Connection _replicaStateConnection;

    ProcessFactory_t _makeProcess;
};

#endif // UPDATERCONNECTIONIMPL_H
