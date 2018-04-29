#ifndef UTILITIES_H
#define UTILITIES_H

#include <QtCore>

namespace logg {
    Q_DECLARE_LOGGING_CATEGORY(basic);
}

namespace updater {
    namespace perm {
        void reduceRObjectNodePermissions(QString const& remoteObjNodeName);
        void reducePriviledge(void);
    }

    void messageHandler(QFile* logFile,
                        QtMsgType msgType,
                        QMessageLogContext const& context,
                        QString const& msg);
}

#endif // LOG_H
