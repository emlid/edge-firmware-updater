#include <QtTest>

#include "impl/UpdaterConnectionImpl.h"
#include "UpdaterConnection.h"

// add necessary includes here

namespace predefs {
    static const auto updaterRelativePath = "../../main/fwupgrader";
    static const auto updaterFileInfo = QFileInfo(updaterRelativePath);

    auto makeDefaultProcess(void) {
        return std::unique_ptr<client::UpdaterProcess> {
            new client::UpdaterProcess()
        };
    }

    auto makeUpdaterConnection(
         client::UpdaterConnectionImpl::ProcessFactory_t procFactory = makeDefaultProcess
    )
    {
        return std::unique_ptr<client::UpdaterConnection>(
            new client::UpdaterConnectionImpl(procFactory)
        );
    }
}


class UpdaterConnection_test : public QObject
{
    Q_OBJECT

public:
    UpdaterConnection_test() { }
    ~UpdaterConnection_test() = default;

    void dirTest(QString const& dirName);

 private slots:
    void case_initialState(void);
    void case_arguments(void);
    void case_connection(void);
    void case_checkAsyncConnection(void);
    void case_checkAbort(void);
    void case_checkOnAsyncDisconnect(void);
    void case_checkWithoutHeartbeat(void);
    void case_checkWithHeartbeat(void);
    void case_checkSpacesInPath(void);
    void case_checkQuotesInPath(void);
};


void UpdaterConnection_test::case_initialState(void)
{
    auto connection = predefs::makeUpdaterConnection();
    QCOMPARE(connection->currentState(), client::UpdaterConnection::Disconneted);
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
    QCOMPARE(connection->currentState(), client::UpdaterConnection::Disconneted);

    connection->sever();
    QVERIFY(connection->currentState() != client::UpdaterConnection::Disconnecting);
}


void UpdaterConnection_test::case_connection(void)
{
    auto connection = predefs::makeUpdaterConnection();

    QSignalSpy checkConnection(connection.get(), &client::UpdaterConnection::established);

    qInfo() << "You will see window with username and passwd. Please, fill these fields";
    connection->establish(predefs::updaterFileInfo.absoluteFilePath());
    qInfo() << "Connecting...";

    QCOMPARE(connection->currentState(), client::UpdaterConnection::Connecting);
    QVERIFY(checkConnection.wait());
    QVERIFY(connection->isEstablished());
    QCOMPARE(connection->currentState(), client::UpdaterConnection::Established);

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

    QSignalSpy checkDisconnection(connection.get(), &client::UpdaterConnection::disconnected);
    connection->sever();

    QCOMPARE(connection->currentState(), client::UpdaterConnection::Disconnecting);
    QVERIFY(checkDisconnection.wait());
    QVERIFY(!connection->isEstablished());
    QCOMPARE(connection->currentState(), client::UpdaterConnection::Disconneted);

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
    QSignalSpy checkConnection(connection.get(), &client::UpdaterConnection::established);
    connection->establish(predefs::updaterFileInfo.absoluteFilePath());

    QVERIFY(checkConnection.wait());
    connection.reset();
}


void UpdaterConnection_test::case_checkAbort(void)
{
    auto proc = new client::UpdaterProcess;
    auto mocProcessFactory =
        [proc] (void) { return std::unique_ptr<client::UpdaterProcess>(proc); };
    auto connection = predefs::makeUpdaterConnection(mocProcessFactory);

    connection->establish(predefs::updaterFileInfo.absoluteFilePath());
    QCOMPARE(connection->currentState(), client::UpdaterConnection::Connecting);

    proc->kill();

    QSignalSpy checkState(connection.get(), &client::UpdaterConnection::stateChanged);

    QVERIFY(checkState.wait());
    QCOMPARE(connection->currentState(), client::UpdaterConnection::Aborted);
}


void UpdaterConnection_test::case_checkOnAsyncDisconnect(void)
{
    // check on some troubles with deleting connection in "Disconnecting" state

    auto connection = predefs::makeUpdaterConnection();
    QSignalSpy checkConnection(connection.get(), &client::UpdaterConnection::established);
    connection->establish(predefs::updaterFileInfo.absoluteFilePath());

    QVERIFY(checkConnection.wait());
    connection->sever();
    connection.reset();
}


void UpdaterConnection_test::case_checkWithoutHeartbeat(void)
{
    // If we disable heartbeat, firmware updater should
    // terminate yourself

    qInfo() << "Make custom config without heartbeat";

    auto config = client::UpdaterConnectionImpl::Config::defaultConfig();
    config.enableHeartbeat = false;

    auto connection = std::unique_ptr<client::UpdaterConnection>(
        new client::UpdaterConnectionImpl(predefs::makeDefaultProcess, config)
    );

    qInfo() << "Waiting for connection...";

    QSignalSpy checkConnection(connection.get(), &client::UpdaterConnection::established);
    QSignalSpy checkDisconnection(connection.get(), &client::UpdaterConnection::stateChanged);

    connection->establish(predefs::updaterFileInfo.absoluteFilePath());
    QVERIFY(checkConnection.wait());
    qInfo() << "Connected. Waiting for disconnection...";

    QVERIFY(checkDisconnection.wait());
    QCOMPARE(connection->currentState(), client::UpdaterConnection::Aborted);
    qInfo() << "Disconnected.";
}


void UpdaterConnection_test::case_checkWithHeartbeat(void)
{
    qInfo() << "Waiting for connection...";

    auto connection = predefs::makeUpdaterConnection();

    QSignalSpy checkConnection(connection.get(), &client::UpdaterConnection::established);
    QSignalSpy checkDisconnection(connection.get(), &client::UpdaterConnection::stateChanged);

    connection->establish(predefs::updaterFileInfo.absoluteFilePath());
    QVERIFY(checkConnection.wait());
    qInfo() << "Connected.";

    auto waitTime = updater::shared::properties.heartbeatPeriod * 5;
    QVERIFY(!checkDisconnection.wait(waitTime));
    QCOMPARE(connection->currentState(), client::UpdaterConnection::Established);
    qInfo() << "Not disconnected.";
}


void UpdaterConnection_test::dirTest(QString const& dirName)
{
    const struct {
        QString tempDir;
        QString dirWithSpaces;
    } dirNames = {"temp", dirName};

    auto workDir = QDir::home();

    workDir.mkdir(dirNames.tempDir);
    workDir.cd(dirNames.tempDir);

    auto temporaryDirPath = workDir.absolutePath();

    workDir.mkdir(dirNames.dirWithSpaces);
    workDir.cd(dirNames.dirWithSpaces);

    auto tmpExeName = predefs::updaterFileInfo.fileName();
    auto tmpExePath = workDir.absolutePath() + '/' + tmpExeName;
    QFile::copy(predefs::updaterFileInfo.absoluteFilePath(), tmpExePath);

    qInfo() << QString("Firmware updater moved from %1 to %2")
                   .arg(predefs::updaterFileInfo.absoluteFilePath())
                   .arg(tmpExePath);

    qInfo() <<  "Trying to establish connection with updater copy";

    auto connection = predefs::makeUpdaterConnection();

    QSignalSpy checkConnection(connection.get(), &client::UpdaterConnection::established);

    qInfo() << "You will see window with username and passwd. Please, fill these fields";
    connection->establish(tmpExePath);
    qInfo() << "Connecting...";

    QCOMPARE(connection->currentState(), client::UpdaterConnection::Connecting);
    QVERIFY(checkConnection.wait());
    QVERIFY(connection->isEstablished());
    QCOMPARE(connection->currentState(), client::UpdaterConnection::Established);

    workDir.cd(temporaryDirPath);
    workDir.removeRecursively();

}


void UpdaterConnection_test::case_checkSpacesInPath(void)
{
    dirTest("dir with spaces");
}


void UpdaterConnection_test::case_checkQuotesInPath(void)
{
    dirTest("dir'with\"quotes");
}

QTEST_MAIN(UpdaterConnection_test)

#include "UpdaterConnection_test.moc"
