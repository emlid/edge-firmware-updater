QT += testlib remoteobjects
QT -= gui

CONFIG += qt console warn_on depend_includepath testcase c++14
CONFIG -= app_bundle

TEMPLATE = app

SOURCES += \
    UpdaterConnection_test.cpp

win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../../client/release/ -lclient
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../../client/debug/ -lclient
else:unix: LIBS += -L$$OUT_PWD/../../client/ -lclient

INCLUDEPATH += $$PWD/../../client
DEPENDPATH += $$PWD/../../client

win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../../client/release/libclient.a
else:win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../../client/debug/libclient.a
else:win32:!win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../../client/release/client.lib
else:win32:!win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../../client/debug/client.lib
else:unix: PRE_TARGETDEPS += $$OUT_PWD/../../client/libclient.a

win32:include(../../libs/win_deps.pri)
