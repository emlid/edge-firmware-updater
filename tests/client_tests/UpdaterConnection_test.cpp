#include <QtTest>

#include "impl/UpdaterConnectionImpl.h"
#include "UpdaterConnection.h"

// add necessary includes here

namespace predefs {
    static const auto binaryPath = QCoreApplication::applicationDirPath()
            + "../../main/fwupgrader";

    auto makeDefaultProcess(void)
        -> std::unique_ptr<UpdaterProcess>
    {
        return std::unique_ptr<UpdaterProcess> {
            new UpdaterProcess()
        };
    }

    auto makeUpdaterConnection(
         UpdaterConnectionImpl::ProcessFactory_t procFactory = makeDefaultProcess
    ) -> std::unique_ptr<UpdaterConnection>
    {
        return std::unique_ptr<UpdaterConnection>(
            new UpdaterConnectionImpl(procFactory)
        );
    }
}


class UpdaterConnection_test : public QObject
{
    Q_OBJECT

public:
    UpdaterConnection_test() { }
    ~UpdaterConnection_test() = default;

private slots:
    void case_initialState(void);
    void case_arguments(void);
    void case_connection(void);
    void case_checkAsyncConnection(void);
    void case_checkAbort(void);
    void case_checkOnAsyncDisconnect(void);
    void case_checkWithoutHeartbeat(void);
    void case_checkWithHeartbeat(void);
};


void UpdaterConnection_test::case_initialState(void)
{
    auto connection = predefs::makeUpdaterConnection();
    QCOMPARE(connection->currentState(), UpdaterConnection::Disconneted);
    QVERIFY(connection->detailedErrorDescription().isEmpty());
    QVERIFY(!connection->isEstablished());

    auto updaterWeakPtr = connection->updater();
    auto updater = updaterWeakPtr.lock();

    QCOMPARE(updater.get(), nullptr);
}


void UpdaterConnection_test::case_arguments(void)
{
    auto connection = predefs::makeUpdaterConnection();

    connection->establish("");
    QCOMPARE(connection->currentState(), UpdaterConnection::Disconneted);

    connection->sever();
    QVERIFY(connection->currentState() != UpdaterConnection::Disconnecting);
}


void UpdaterConnection_test::case_connection(void)
{
    auto connection = predefs::makeUpdaterConnection();

    QSignalSpy checkConnection(connection.get(), &UpdaterConnection::established);

    qInfo() << "You will see window with username and passwd. Please, fill these fields";
    connection->establish(predefs::binaryPath);
    qInfo() << "Connecting...";

    QCOMPARE(connection->currentState(), UpdaterConnection::Connecting);
    QVERIFY(checkConnection.wait());
    QVERIFY(connection->isEstablished());
    QCOMPARE(connection->currentState(), UpdaterConnection::Established);

    // check updater
    auto updaterWeakPtr = connection->updater();
    auto updater = updaterWeakPtr.lock();

    qInfo() << "Successfully connected";

    QVERIFY(updater.get());
    if (updater) {
        QVERIFY(updater->isValid());
        qInfo() << "Updater object is valid";
        updater.reset();
    }

    qInfo() << "Disconnecting...";

    QSignalSpy checkDisconnection(connection.get(), &UpdaterConnection::disconnected);
    connection->sever();

    QCOMPARE(connection->currentState(), UpdaterConnection::Disconnecting);
    QVERIFY(checkDisconnection.wait());
    QVERIFY(!connection->isEstablished());
    QCOMPARE(connection->currentState(), UpdaterConnection::Disconneted);

    qInfo() << "Successfully disconnected";

    updater = updaterWeakPtr.lock();
    QVERIFY(updater.get());
    if (updater.get()) {
        QVERIFY(!updater->isValid());
        qInfo() << "Updater is invalid after disconnect";
    }
}


void UpdaterConnection_test::case_checkAsyncConnection(void)
{
    // check on troubles with deleting in "established" state

    auto connection = predefs::makeUpdaterConnection();
    QSignalSpy checkConnection(connection.get(), &UpdaterConnection::established);
    connection->establish(predefs::binaryPath);

    QVERIFY(checkConnection.wait());
    connection.reset();
}


void UpdaterConnection_test::case_checkAbort(void)
{
    auto proc = new UpdaterProcess;
    auto mocProcessFactory =
        [proc] (void) { return std::unique_ptr<UpdaterProcess>(proc); };
    auto connection = predefs::makeUpdaterConnection(mocProcessFactory);

    connection->establish(predefs::binaryPath);
    QCOMPARE(connection->currentState(), UpdaterConnection::Connecting);

    proc->kill();

    QSignalSpy checkState(connection.get(), &UpdaterConnection::stateChanged);

    QVERIFY(checkState.wait());
    QCOMPARE(connection->currentState(), UpdaterConnection::Aborted);
}


void UpdaterConnection_test::case_checkOnAsyncDisconnect(void)
{
    // check on some troubles with deleting connection in "Disconnecting" state

    auto connection = predefs::makeUpdaterConnection();
    QSignalSpy checkConnection(connection.get(), &UpdaterConnection::established);
    connection->establish(predefs::binaryPath);

    QVERIFY(checkConnection.wait());
    connection->sever();
    connection.reset();
}


void UpdaterConnection_test::case_checkWithoutHeartbeat(void)
{
    // If we disable heartbeat, firmware updater should
    // terminate yourself

    qInfo() << "Make custom config without heartbeat";

    auto config = UpdaterConnectionImpl::Config::defaultConfig();
    config.enableHeartbeat = false;

    auto connection = std::unique_ptr<UpdaterConnection>(
        new UpdaterConnectionImpl(predefs::makeDefaultProcess, config)
    );

    qInfo() << "Waiting for connection...";

    QSignalSpy checkConnection(connection.get(), &UpdaterConnection::established);
    QSignalSpy checkDisconnection(connection.get(), &UpdaterConnection::stateChanged);

    connection->establish(predefs::binaryPath);
    QVERIFY(checkConnection.wait());
    qInfo() << "Connected. Waiting for disconnection...";

    QVERIFY(checkDisconnection.wait());
    QCOMPARE(connection->currentState(), UpdaterConnection::Aborted);
    qInfo() << "Disconnected.";
}


void UpdaterConnection_test::case_checkWithHeartbeat(void)
{
    qInfo() << "Waiting for connection...";

    auto connection = predefs::makeUpdaterConnection();

    QSignalSpy checkConnection(connection.get(), &UpdaterConnection::established);
    QSignalSpy checkDisconnection(connection.get(), &UpdaterConnection::stateChanged);

    connection->establish(predefs::binaryPath);
    QVERIFY(checkConnection.wait());
    qInfo() << "Connected.";

    QVERIFY(!checkDisconnection.wait(10000));
    QCOMPARE(connection->currentState(), UpdaterConnection::Established);
    qInfo() << "Not disconnected.";
}

QTEST_MAIN(UpdaterConnection_test)

#include "UpdaterConnection_test.moc"
