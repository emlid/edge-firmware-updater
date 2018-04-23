#ifndef UPDATERPROCESS_H
#define UPDATERPROCESS_H

#include <QtCore>

namespace client {
    class UpdaterProcess;
}

class client::UpdaterProcess : public QProcess
{
    Q_OBJECT
public:
    UpdaterProcess();

    void startAsAdmin(QString const& program,
                      QStringList const& args = {},
                      OpenMode const& mode = ReadWrite);
};

#endif // UPDATERPROCESS_H
