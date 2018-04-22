#ifndef UPDATERCONNECTIONIMPL_H
#define UPDATERCONNECTIONIMPL_H

#include <QtCore>
#include <QtRemoteObjects>

#include <memory>

#include "UpdaterConnection.h"
#include "UpdaterProcess.h"

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
        bool verbose;

        constexpr static Config defaultConfig(void) {
            return {1500, 1500, true};
        }
    };

    UpdaterConnectionImpl(ProcessFactory_t const& procFactory,
                      Config const& config = Config::defaultConfig(),
                      QObject* parent = nullptr);
    ~UpdaterConnectionImpl(void) override;

private slots:
    void _handleProcessFinished(int exitCode, UpdaterProcess::ExitStatus);
    void _connectToUpdaterNode(void);

private:
    virtual void _establish(QString const& serverNodeName,
                            QString const& updaterExePath) override;
    virtual void _sever(void) override;


    Config     _config;
    QString    _updaterNodeName;

    QRemoteObjectNode _node;
    QTimer _disconnectTimer;

    std::shared_ptr<EdgeFirmwareUpdaterIPCReplica> _replica;
    std::unique_ptr<UpdaterProcess> _proc;

    QMetaObject::Connection _processStateConnection;
    QMetaObject::Connection _replicaStateConnection;

    ProcessFactory_t _makeProcess;
};

#endif // UPDATERCONNECTIONIMPL_H
