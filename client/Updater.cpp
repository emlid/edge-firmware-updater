#include "Updater.h"

static void warn(QString const& action)
{
    qWarning() << QString("trying to perform %1 with loaded or invalid updater")
                  .arg(action);
}


static void registerMetatypes(void)
{
    static auto registered = false;
    if (!registered) {
        qRegisterMetaType<Updater::State>("UpdaterBase::State");
    }
}


Updater::Updater(QObject* parent)
    : QObject(parent),
      _state(State::Invalid)
{
    registerMetatypes();
}


bool Updater::isValid(void)
{
    return _state != State::Invalid;
}


bool Updater::isIdle(void)
{
    return _state == State::Idle;
}


bool Updater::initializeDevice(void)
{
    // precheck

    if (!isValid() || !isIdle()) {
        warn("Initialization");
        return false;
    }

    return _initializeDevice();
}


bool Updater::flash(QString const& firmwarePath, bool checksumEnabled)
{
    // precheck

    if (!QFileInfo(firmwarePath).exists()) {
        qWarning() << QString("firmware %1 isn't exist").arg(firmwarePath);
        return false;
    }

    if (!isValid() || !isIdle()) {
        warn("Upgrading");
        return false;
    }

    return _flash(firmwarePath, checksumEnabled);
}


bool Updater::cancel(void)
{
    // precheck

    if (!isValid()) {
        warn("Cancellation");
        return false;
    }

    return _cancel();
}


bool Updater::finish(void)
{
    if (!isValid()) {
        warn("Exiting");
        return false;
    }

    return _finish();
}


void Updater::_changeState(State targetState)
{
    auto oldState = _state;
    _state = targetState;
    emit stateChanged(_state, oldState);
}
