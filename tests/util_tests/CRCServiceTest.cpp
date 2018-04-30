#include "CRCServiceTest.h"

#include <QtTest>
#include <QBuffer>

#include "../../util/CancellationPoint.h"
#include "../../util/impl/CRCServiceImpl.h"

CRCServiceTest::CRCServiceTest()
    : _crcService(new util::CRCServiceImpl())
{ }


void CRCServiceTest::checkWithSmallBuffer_testcase(void)
{
    QByteArray data = "Some data";
    QBuffer buf1(&data);
    QBuffer buf2(&data);

    buf1.open(QIODevice::ReadOnly);
    buf2.open(QIODevice::ReadOnly);

    auto result1 = _crcService->computeCRC(&buf1, buf1.size());
    QCOMPARE(result1.second, util::CRCServiceImpl::RetStatus::Success);

    auto result2 = _crcService->computeCRC(&buf2, buf2.size());

    QCOMPARE(result2.second, util::IOService::RetStatus::Success);
    QCOMPARE(result1.first, result2.first);
}


void CRCServiceTest::checkFails_testcase(void)
{
    QByteArray data = "Some data";
    QByteArray data2 = "Some data2";

    QBuffer buf(&data);
    QBuffer buf2(&data2);

    buf.open(QIODevice::ReadOnly);
    buf2.open(QIODevice::ReadOnly);

    auto result = _crcService->computeCRC(&buf, buf.size());
    auto result2 = _crcService->computeCRC(&buf2, buf2.size());

    QCOMPARE(result.second, util::IOService::RetStatus::Success);
    QVERIFY(!result.first.isEmpty());
    QVERIFY(result2.first != result.first);
    QCOMPARE(result2.second, result.second);
}

void CRCServiceTest::checkCancellation_testcase(void)
{
    auto sourceData = QByteArray("Some data");

    QBuffer inputBuffer(&sourceData);
    inputBuffer.open(QIODevice::ReadOnly);

    auto cpoint = util::CancellationPoint();

    auto future = std::async([this] (auto in, auto size, auto cp) {
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
            return _crcService->computeCRC(in, size, [cp] () { return cp->isCancellationRequested(); });
        }, &inputBuffer, inputBuffer.size(), &cpoint
    );

    cpoint.cancel();
    future.wait();

    QCOMPARE(future.get().second, util::IOService::RetStatus::Cancelled);
}
