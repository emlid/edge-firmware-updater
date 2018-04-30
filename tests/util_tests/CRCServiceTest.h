#ifndef CRCSERVICETEST_H
#define CRCSERVICETEST_H

#include <future>
#include "../../util/CRCService.h"

class CRCServiceTest : public QObject
{
    Q_OBJECT
public:
    CRCServiceTest(void);
    ~CRCServiceTest(void) = default;

private slots:
    void checkWithSmallBuffer_testcase(void);
    void checkFails_testcase(void);
    void checkCancellation_testcase(void);

private:
    std::unique_ptr<util::ICRCService> _crcService;
};

#endif

