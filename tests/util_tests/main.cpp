#include "FlashingServiceTest.h"
#include "CRCServiceTest.h"
#include "PollTest.h"

#include <QTest>

int main(int argc, char *argv[])
{
    auto status = 0;
    FlashingServiceTest flashingTest;
    status |= QTest::qExec(&flashingTest, argc, argv);

    CRCServiceTest crcTest;
    status |= QTest::qExec(&crcTest, argc, argv);

    PollTest pollTest;
    status |= QTest::qExec(&pollTest, argc, argv);

    return status;
}
