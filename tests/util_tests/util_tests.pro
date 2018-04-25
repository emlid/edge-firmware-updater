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

include(../../util/src.pri)
win32:include(../../libs/win_deps.pri)

HEADERS += \
    FlashingServiceTest.h \
    CRCServiceTest.h \
    PollTest.h
