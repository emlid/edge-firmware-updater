#ifndef UPDATER_H
#define UPDATER_H

#include <QtCore>

class Updater : public QObject
{
    Q_OBJECT
public:
    enum State {
        Initializing,
        Flashing,
        ComputingChecksum,
        Idle,
        Invalid
    };
    Q_ENUM(State)

    explicit Updater(QObject* parent = nullptr);
    virtual ~Updater() = default;

    bool isValid(void);
    bool isIdle(void);

public slots:
    bool initializeDevice(void);
    bool flash(QString const& fwImagePath, bool checksum);
    bool cancel(void);
    bool finish(void);

signals:
    void stateChanged(State state, State old);
    void deviceInitialized(bool status);
    void deviceFlashed(bool status);
    void cancelled(void);

    void updateProgressChanged(uint value);
    void firmwareVersion(QString const& version);

    void errorMessageReceived (QString const& msg);
    void infoMessageReceived  (QString const& msg);
    void warnMessageReceived  (QString const& msg);

private:
    virtual bool _initializeDevice(void) = 0;
    virtual bool _flash(QString const& fwImagePath, bool checksum) = 0;
    virtual bool _cancel(void) = 0;
    virtual bool _finish(void) = 0;

protected:
    void _changeState(State targetState);

private:
    State _state;
};

#endif // UPDATER_H
