QT -= gui

CONFIG += c++14 console
CONFIG -= app_bundle

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += main.cpp





win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../../edge/release/ -ledge
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../../edge/debug/ -ledge
else:unix: LIBS += -L$$OUT_PWD/../../edge/ -ledge

INCLUDEPATH += $$PWD/../../edge
DEPENDPATH += $$PWD/../../edge

win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../../edge/release/libedge.a
else:win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../../edge/debug/libedge.a
else:win32:!win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../../edge/release/edge.lib
else:win32:!win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../../edge/debug/edge.lib
else:unix: PRE_TARGETDEPS += $$OUT_PWD/../../edge/libedge.a


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


win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../../devlib/devlib/release/ -ldevlib
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../../devlib/devlib/debug/ -ldevlib
else:unix: LIBS += -L$$OUT_PWD/../../devlib/devlib/ -ldevlib

INCLUDEPATH += $$PWD/../../devlib/devlib
DEPENDPATH += $$PWD/../../devlib/devlib

win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../../devlib/devlib/release/libdevlib.a
else:win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../../devlib/devlib/debug/libdevlib.a
else:win32:!win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../../devlib/devlib/release/devlib.lib
else:win32:!win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../../devlib/devlib/debug/devlib.lib
else:unix: PRE_TARGETDEPS += $$OUT_PWD/../../devlib/devlib/libdevlib.a


include(../../edge/edge_deps.pri)
win32:include(../../libs/win_deps.pri)
