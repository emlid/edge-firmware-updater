#ifndef POLLTEST_H
#define POLLTEST_H


#include <future>
#include "../../util/FlashingService.h"

// add necessary includes here

class PollTest : public QObject
{
    Q_OBJECT

public:
    PollTest(void);
    ~PollTest(void) = default;

private slots:
    void false_testcase(void);
    void true_testcase(void);

};


#endif // POLLTEST_H
