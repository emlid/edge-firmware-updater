#ifndef UPDATERCONNECTION_H
#define UPDATERCONNECTION_H

#include <QtCore>

#include "Updater.h"

class UpdaterConnection : public QObject
{
    Q_OBJECT
public:
    enum State {
        Disconnecting,
        Disconneted,
        Aborted,
        Connecting,
        Established,
        Errored
    };
    Q_ENUM(State)

    virtual ~UpdaterConnection(void) = default;

    auto detailedErrorDescription(void) const -> QString {
        return _errorString;
    }

    auto currentState(void) const -> State {
        return _state;
    }

    auto isEstablished(void) const -> bool {
        return _state == State::Established;
    }

    auto updater(void) -> std::weak_ptr<Updater> {
        return _updater;
    }

    void establish(QString const& updaterExePath) {
        auto updaterExeInfo = QFileInfo(updaterExePath);
        if (!updaterExeInfo.exists() || !updaterExeInfo.isExecutable()) {
            qWarning() << "Updater executable file is not exist or not executable.";
            return;
        }

        switch (_state) {
            case State::Established:
                qWarning() << "Connection already established";
                return;

            case State::Disconnecting:
                qWarning() << "Connection is disconnecting";
                return;

            case State::Connecting:
                qWarning() << "Connection is establishing";
                return;

            default:
                _changeState(State::Connecting);
                return _establish(updaterExePath);
        }
    }

    void sever(void) {
        if (_state != State::Established) {
            qWarning() << "Trying to sever unconnected connection";
            return;
        }

        _changeState(State::Disconnecting);
        return _sever();
    }

signals:
    void stateChanged(State current, State old);
    void established(void);
    void disconnected(void);

protected:
    UpdaterConnection(QObject* parent = nullptr)
        : QObject(parent),
          _state(State::Disconneted)
    {
        _registerMetatypes();
    }

    void _changeState(State const& state) {
        auto old = _state;
        _state = state;
        emit stateChanged(state, old);

        if (_state == State::Established) {
            emit established();
        } else if (_state == State::Disconneted) {
            emit disconnected();
        }
    }

    void _setErrorString(QString const& description) {
        _errorString = description;
    }

    std::shared_ptr<Updater> _updater;

private:
    static void _registerMetatypes(void)
    {
        static auto registered = false;
        if (!registered) {
            qRegisterMetaType<UpdaterConnection::State>("UpdaterConnectionBase::State");
        }
    }

    virtual void _establish(QString const& updaterExePath) = 0;
    virtual void _sever(void) = 0;

    volatile State               _state;
    QString                      _errorString;
};

#endif // UPDATERCONNECTION_H
