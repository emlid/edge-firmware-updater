#include "FlashingServiceTest.h"
#include <QIODevice>
#include <QtTest>

#include "../../util/CancellationPoint.h"
#include "../../util/impl/FlashingServiceImpl.h"

FlashingServiceTest::FlashingServiceTest()
    : _flashingService(new util::FlashingServiceImpl())
{ }

void FlashingServiceTest::checkWithSmallBuffer_testcase(void)
{
    auto sourceData = QByteArray("Some firmware");
    auto destData = QByteArray();

    QBuffer inputBuffer(&sourceData);
    QBuffer outputBuffer(&destData);

    inputBuffer.open(QIODevice::ReadOnly);
    outputBuffer.open(QIODevice::WriteOnly);

    auto status = _flashingService->flash(&inputBuffer, &outputBuffer);

    QCOMPARE(status, util::IFlashingService::RetStatus::Success);
    QCOMPARE(destData.size(), sourceData.size());
    QCOMPARE(destData, sourceData);
}


void FlashingServiceTest::checkWithEmptyBuffer_testcase(void)
{
    auto sourceData = QByteArray();
    auto destData = QByteArray("Some data");
    auto cache = destData;

    QBuffer inputBuffer(&sourceData);
    QBuffer outputBuffer(&destData);

    inputBuffer.open(QIODevice::ReadOnly);
    outputBuffer.open(QIODevice::WriteOnly);

    auto status = _flashingService->flash(&inputBuffer, &outputBuffer);
    QCOMPARE(status, util::IFlashingService::RetStatus::Success);
    QCOMPARE(destData, cache);
}


void FlashingServiceTest::checkCancellation_testcase(void)
{
    auto sourceData = QByteArray("Some data");
    auto destData = QByteArray("Some data");

    QBuffer inputBuffer(&sourceData);
    QBuffer outputBuffer(&destData);

    inputBuffer.open(QIODevice::ReadOnly);
    outputBuffer.open(QIODevice::WriteOnly);

    auto cpoint = util::CancellationPoint();

    auto future = std::async([this] (auto in, auto out, auto cp) {
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
            return _flashingService->flash(in, out, [cp] () { return cp->isCancellationRequested(); } );
        }, &inputBuffer, &outputBuffer, &cpoint
    );

    cpoint.cancel();
    future.wait();

    QCOMPARE(future.get(), util::IFlashingService::RetStatus::Cancelled);
}
