QT += testlib
QT -= gui

CONFIG += c++14 qt console warn_on depend_includepath testcase
CONFIG -= app_bundle

TEMPLATE = app

SOURCES += \ 
    main.cpp \
    FlashingServiceTest.cpp \
    CRCServiceTest.cpp \
    PollTest.cpp


HEADERS += \
    FlashingServiceTest.h \
    CRCServiceTest.h \
    PollTest.h

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

win32:include(../../libs/win_deps.pri)
