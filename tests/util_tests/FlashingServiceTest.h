#ifndef FLASHINGSERVICETEST_H
#define FLASHINGSERVICETEST_H

#include <future>
#include "../../util/FlashingService.h"

// add necessary includes here

class FlashingServiceTest : public QObject
{
    Q_OBJECT

public:
    FlashingServiceTest(void);
    ~FlashingServiceTest(void) = default;

private slots:
    void checkWithSmallBuffer_testcase(void);
    void checkWithEmptyBuffer_testcase(void);
    void checkCancellation_testcase(void);

private:
    std::unique_ptr<util::IFlashingService> _flashingService;
};


#endif
