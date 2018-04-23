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
        qRegisterMetaType<client::Updater::State>("UpdaterBase::State");
    }
}


client::Updater::Updater(QObject* parent)
    : QObject(parent),
      _state(State::Invalid)
{
    registerMetatypes();
}


bool client::Updater::isValid(void)
{
    return _state != State::Invalid;
}


bool client::Updater::isIdle(void)
{
    return _state == State::Idle;
}


bool client::Updater::initializeDevice(void)
{
    // precheck

    if (!isValid() || !isIdle()) {
        warn("Initialization");
        return false;
    }

    return _initializeDevice();
}


bool client::Updater::flash(QString const& firmwarePath, bool checksumEnabled)
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


bool client::Updater::cancel(void)
{
    // precheck

    if (!isValid()) {
        warn("Cancellation");
        return false;
    }

    return _cancel();
}


bool client::Updater::finish(void)
{
    if (!isValid()) {
        warn("Exiting");
        return false;
    }

    return _finish();
}


void client::Updater::_changeState(State targetState)
{
    auto oldState = _state;
    _state = targetState;
    emit stateChanged(_state, oldState);
}
