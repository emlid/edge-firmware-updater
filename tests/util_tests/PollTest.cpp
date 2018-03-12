#include "PollTest.h"
#include "../../util/util.h"

#include <QTest>

PollTest::PollTest(void) {}

void PollTest::false_testcase(void)
{
    auto nullFunc = [] (void) -> std::unique_ptr<int> { return {}; };

    auto result = util::poll<std::unique_ptr<int>>(nullFunc);
    QCOMPARE(result.get(), nullptr);
}


void PollTest::true_testcase(void)
{
    auto someFunc = [] (void) -> std::unique_ptr<int> { return std::make_unique<int>(1); };
    auto result = util::poll<std::unique_ptr<int>>(someFunc);
    QCOMPARE(*(result.get()), 1);
}
