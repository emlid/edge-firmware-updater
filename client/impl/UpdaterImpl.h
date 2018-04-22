#ifndef UPDATERIMPL_H
#define UPDATERIMPL_H

#include <QtCore>
#include "Updater.h"

#include <memory>

class EdgeFirmwareUpdaterIPCReplica;

// The class has asynchronous API
class UpdaterImpl : public Updater
{
    Q_OBJECT
public:

    enum FinishStatus {
        Succeed = 1,
        Cancelled = 0,
        Failed = -1
    };
    Q_ENUM(FinishStatus)

    enum LogMsgType {
        Info = 0,
        Warning,
        Error
    };
    Q_ENUM(LogMsgType)

    explicit UpdaterImpl(std::shared_ptr<EdgeFirmwareUpdaterIPCReplica> updaterReplica,
                         QObject* parent = nullptr);
    ~UpdaterImpl(void) override;

private:
    bool _initializeDevice(void) override;
    bool _flash(QString const& fwImagePath, bool checksum) override;
    bool _cancel(void) override;
    bool _finish(void) override;

    void _handleReplicaLogMessage(QString msg, int type);

    void _attachToUpdaterReplica(void);
    void _makeProxies(void);

    void _openUpdateSession(void);
    void _closeUpdateSession(void);

private:
    bool  _sessionOpened;
    std::shared_ptr<EdgeFirmwareUpdaterIPCReplica> _updaterReplica;
};

#endif // UPDATERIMPL_H
