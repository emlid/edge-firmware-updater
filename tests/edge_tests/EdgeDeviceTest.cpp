#include <QtTest>
#include <QTest>

#include "edge.h"
#include "impl/EdgeDeviceImpl.h"
#include "shared.h"
#include "devlib.h"
#include "rpi.h"

// add necessary includes here


class MountpointMock : public devlib::IMountpoint
{
public:
    MountpointMock(bool ismounted, QString const& fsPath)
        : _isMounted(ismounted), _fsPath(fsPath)
    { }

    ~MountpointMock(void) = default;

private:
    virtual auto isMounted_core(void) const -> bool override { return _isMounted; }
    virtual auto fsPath_core(void) const -> QString const& override { return _fsPath; }
    virtual auto umount_core(void) -> std::unique_ptr<devlib::IMountpointLock> override
    {
        class LockMock : public devlib::IMountpointLock {
            bool locked_core(void) const override { return true; }
            void release_core(void) override { }
        };

        return std::make_unique<LockMock>();
    }

    bool _isMounted;
    QString _fsPath;
};

class PartitionMock: public devlib::IPartition
{
public:
    PartitionMock(QString const& filePath,
                  QString const& label)
        : _filePath(filePath),
          _label(label) { }

private:
    virtual QString filePath_core(void) const noexcept override { return _filePath; }
    virtual QString label_core(void) const noexcept override { return _label; }

    virtual std::unique_ptr<devlib::IMountpoint> mount_core(QString const& path)  override {
        return std::make_unique<MountpointMock>(true, path);
    }

    virtual std::vector<std::unique_ptr<devlib::IMountpoint>> mountpoints_core(void) override {
        auto list = std::vector<std::unique_ptr<devlib::IMountpoint>>();
        list.push_back(std::make_unique<MountpointMock>(true, "/some/path"));
        return list;
    }

    QString _filePath;
    QString _label;
};

class StorageDeviceInfoMock : public devlib::IStorageDeviceInfo
{
public:
    ~StorageDeviceInfoMock(void) = default;

private:
    virtual int vid_core(void) const noexcept override { return 0x0a5c; }
    virtual int pid_core(void) const noexcept override { return 0x0001; }

    virtual QString filePath_core(void) const noexcept override { return "Some path"; }

    auto mountpoints_core() const
        -> std::vector<std::unique_ptr<devlib::IMountpoint>> override
    {
        std::vector<std::unique_ptr<devlib::IMountpoint>> list;
        list.push_back(std::make_unique<MountpointMock>(true, "/some/path"));
        return list;
    }

    auto partitions_core(void) const
        -> std::vector<std::unique_ptr<devlib::IPartition>> override
    {
        auto list = std::vector<std::unique_ptr<devlib::IPartition>>();
        list.push_back(std::make_unique<PartitionMock>("/some/dev", "boot"));
        return list;
    }
};


class EdgeDeviceTest : public QObject
{
    Q_OBJECT

public:
    EdgeDeviceTest();
    ~EdgeDeviceTest();

private slots:
    void checkVersionParsing_testcase(void);
};

EdgeDeviceTest::EdgeDeviceTest() { }

EdgeDeviceTest::~EdgeDeviceTest() {  }

void EdgeDeviceTest::checkVersionParsing_testcase()
{
    auto config = edge::EdgeConfig(
        0x0a5c, 0x0001,
        0x0001, updater::shared::properties.heartbeatPeriod,
        "issue.txt", "/tmp", "boot"
    );

    auto storageDeviceInfo = std::make_unique<StorageDeviceInfoMock>();
    auto fileFactory = [] (QString const& path) {
        if (path != "/some/path/issue.txt") {
            qWarning() << "Invalid name of file";
        }

        Q_UNUSED(path);

        auto buffer = std::make_unique<QBuffer>(new QByteArray("v1.3"));
        return buffer;
    };

    auto dirFactory = [] (QString const& dir) {
        Q_UNUSED(dir);
        return true;
    };

    edge::EdgeDeviceImpl edgeDevice(
        config, std::move(storageDeviceInfo),
        fileFactory, dirFactory
    );

    auto version = edgeDevice.firmwareVersion();
    QCOMPARE(version, QString("v1.3"));
}

QTEST_APPLESS_MAIN(EdgeDeviceTest)

#include "EdgeDeviceTest.moc"
