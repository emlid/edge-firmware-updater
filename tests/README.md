# Tests
This module contains unit tests and CLI applications for testing other modules

## Naming
```testname_[cli|tests]```
- ``tests`` - for unit-tests
- ``cli`` - for simple cli programs (manual testing)

## How to run

### QtCreator
- Click on the box in the top left corner (generally named ``Projects``)
- Select ``Tests``
- Right click on ``Qt Test`` and ``Run All Tests``

### CLI
- build project as described on main page
- go to ```tests``` directory in build dir
- go to directory with needed test and run the executable
- Example:
```
$ cd tests/edge_tests
$ ./edge_tests
```
- Output:
```
********* Start testing of EdgeDeviceTest *********
Config: Using QtTest library 5.9.1, Qt 5.9.1 (x86_64-little_endian-lp64 shared (dynamic) release build; by Clang 7.0.2 (clang-700.1.81) (Apple))
PASS   : EdgeDeviceTest::initTestCase()
PASS   : EdgeDeviceTest::checkVersionParsing_testcase()
PASS   : EdgeDeviceTest::cleanupTestCase()
Totals: 3 passed, 0 failed, 0 skipped, 0 blacklisted, 1ms
********* Finished testing of EdgeDeviceTest *********
```

