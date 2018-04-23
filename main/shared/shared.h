#ifndef SHARED_H
#define SHARED_H

#include <cstdint>
#include <QString>

namespace updater{

    static auto socketName(void) -> QString {
        #if defined (Q_OS_MACX)
        return QStringLiteral("local:/tmp/fwupg_socket");
        #else
        return QStringLiteral("local:fwupg_socket");
        #endif
    }

    static auto updaterExecutableFilename(void) -> QString {
        auto base = QString("fwugprader");
        #if defined (Q_OS_WIN)
        return base.append(".exe");
        #else
        return base;
        #endif
    }

    namespace shared {
        enum OperationStatus : std::int8_t {
            Succeed = 1,
            Cancelled = 0,
            Failed = -1
        };

        enum LogMessageType : std::int8_t {
            Info = 0,
            Warning,
            Error
        };


        static const struct {
            QString updaterReplicaNodeName;
            QString updaterExecutableFilename;
            int heartbeatPeriod;
            bool heartbeatEnabled;

        } properties = {
            socketName(),
            updaterExecutableFilename(),
            1000,
            true
        };
    }
}

#endif // SHARED_H
