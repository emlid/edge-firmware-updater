QT += testlib
QT -= gui

CONFIG += c++14 qt console warn_on depend_includepath testcase
CONFIG -= app_bundle

TEMPLATE = app

SOURCES += \ 
    EdgeDeviceTest.cpp




win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../../devlib/release/ -ldevlib
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../../devlib/debug/ -ldevlib
else:unix: LIBS += -L$$OUT_PWD/../../devlib/ -ldevlib

INCLUDEPATH += $$PWD/../../devlib
DEPENDPATH += $$PWD/../../devlib

win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../../devlib/release/libdevlib.a
else:win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../../devlib/debug/libdevlib.a
else:win32:!win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../../devlib/release/devlib.lib
else:win32:!win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../../devlib/debug/devlib.lib
else:unix: PRE_TARGETDEPS += $$OUT_PWD/../../devlib/libdevlib.a

win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../../rpi/release/ -lrpi
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../../rpi/debug/ -lrpi
else:unix: LIBS += -L$$OUT_PWD/../../rpi/ -lrpi

INCLUDEPATH += $$PWD/../../rpi
DEPENDPATH += $$PWD/../../rpi

win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../../rpi/release/librpi.a
else:win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../../rpi/debug/librpi.a
else:win32:!win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../../rpi/release/rpi.lib
else:win32:!win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../../rpi/debug/rpi.lib
else:unix: PRE_TARGETDEPS += $$OUT_PWD/../../rpi/librpi.a

win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../../util/release/ -lutil
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../../util/debug/ -lutil
else:unix: LIBS += -L$$OUT_PWD/../../util/ -lutil

INCLUDEPATH += $$PWD/../../util
DEPENDPATH += $$PWD/../../util

win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../../util/release/libutil.a
else:win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../../util/debug/libutil.a
else:win32:!win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../../util/release/util.lib
else:win32:!win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../../util/debug/util.lib
else:unix: PRE_TARGETDEPS += $$OUT_PWD/../../util/libutil.a

SHARED_PATH = ../../main/shared
EDGE_PATH = ../../edge/

include($${SHARED_PATH}/shared.pri)
include($${EDGE_PATH}/edge_deps.pri)
include($${EDGE_PATH}/src.pri)

INCLUDEPATH += $${SHARED_PATH} $${EDGE_PATH}
win32:include(../../libs/win_deps.pri)
