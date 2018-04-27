#include <QtTest>

#include "impl/UpdaterConnectionImpl.h"
#include "UpdaterConnection.h"

// add necessary includes here

namespace predefs {
#ifndef Q_OS_WIN32
    static const auto updaterRelativePath = "../../main/fwupgrader";
#elif defined QT_DEBUG
    static const auto updaterRelativePath = "../../main/debug/fwupgrader.exe";
#else
    static const auto updaterRelativePath = "../../main/release/fwupgrader.exe";
#endif
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
    UpdaterConnection_test() {}
    ~UpdaterConnection_test() = default;

    void pathTest(QString const& dirName);

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
    QCOMPARE(connection->currentState(), client::UpdaterConnection::Errored);

    connection->sever();
    QVERIFY(connection->currentState() != client::UpdaterConnection::Disconnecting);
}


void UpdaterConnection_test::case_connection(void)
{
    auto connection = predefs::makeUpdaterConnection();
    qDebug() << predefs::updaterFileInfo.absoluteFilePath();

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


void UpdaterConnection_test::pathTest(QString const& tempUpdaterName)
{
    auto updaterFileName = predefs::updaterFileInfo.fileName();
    auto updaterDir = predefs::updaterFileInfo.absolutePath();
    auto tempUpdaterPath = updaterDir + '/' + tempUpdaterName;
    QFile::rename(predefs::updaterFileInfo.absoluteFilePath(), tempUpdaterPath);

    qInfo() << QString("Rename updater from %1 to %2")
                   .arg(updaterFileName)
                   .arg(tempUpdaterName);

    qInfo() <<  "Trying to establish connection with updater";

    auto connection = predefs::makeUpdaterConnection();

    QSignalSpy checkConnection(connection.get(),
                               &client::UpdaterConnection::established);

    // reset original name
    QObject::connect(connection.get(), &client::UpdaterConnection::destroyed,
        [tempUpdaterPath, updaterDir, updaterFileName] (void) {
            QFile::rename(tempUpdaterPath, updaterDir + '/' + updaterFileName);
        }
    );

    qInfo() << "You will see window with username and passwd. Please, fill these fields";
    connection->establish(tempUpdaterPath);
    qInfo() << "Connecting...";

    QCOMPARE(connection->currentState(), client::UpdaterConnection::Connecting);
    QVERIFY(checkConnection.wait());
    QVERIFY(connection->isEstablished());
    QCOMPARE(connection->currentState(), client::UpdaterConnection::Established);
}


void UpdaterConnection_test::case_checkSpacesInPath(void)
{
#ifdef Q_OS_WIN
    pathTest("up date r.exe");
#else
    pathTest("up date r");
#endif
}


void UpdaterConnection_test::case_checkQuotesInPath(void)
{
#ifdef Q_OS_WIN
    pathTest("up'dater.exe");
#else
    pathTest("up'dater");
#endif
}

QTEST_MAIN(UpdaterConnection_test)

#include "UpdaterConnection_test.moc"
