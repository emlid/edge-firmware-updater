# edge library
win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../edge/release/ -ledge
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../edge/debug/ -ledge
else:unix: LIBS += -L$$OUT_PWD/../edge/ -ledge

INCLUDEPATH += $$PWD/../edge
DEPENDPATH += $$PWD/../edge

win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../edge/release/libedge.a
else:win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../edge/debug/libedge.a
else:win32:!win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../edge/release/edge.lib
else:win32:!win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../edge/debug/edge.lib
else:unix: PRE_TARGETDEPS += $$OUT_PWD/../edge/libedge.a

# util library
win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../util/release/ -lutil
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../util/debug/ -lutil
else:unix: LIBS += -L$$OUT_PWD/../util/ -lutil

INCLUDEPATH += $$PWD/../util
DEPENDPATH += $$PWD/../util

win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../util/release/libutil.a
else:win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../util/debug/libutil.a
else:win32:!win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../util/release/util.lib
else:win32:!win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../util/debug/util.lib
else:unix: PRE_TARGETDEPS += $$OUT_PWD/../util/libutil.a

# rpi library
win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../rpi/release/ -lrpi
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../rpi/debug/ -lrpi
else:unix: LIBS += -L$$OUT_PWD/../rpi/ -lrpi

INCLUDEPATH += $$PWD/../rpi
DEPENDPATH += $$PWD/../rpi

win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../rpi/release/librpi.a
else:win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../rpi/debug/librpi.a
else:win32:!win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../rpi/release/rpi.lib
else:win32:!win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../rpi/debug/rpi.lib
else:unix: PRE_TARGETDEPS += $$OUT_PWD/../rpi/librpi.a

# devlib library
win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../devlib/devlib/release/ -ldevlib
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../devlib/devlib/debug/ -ldevlib
else:unix: LIBS += -L$$OUT_PWD/../devlib/devlib/ -ldevlib

INCLUDEPATH += $$PWD/../devlib/devlib
DEPENDPATH += $$PWD/../devlib/devlib

win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../devlib/devlib/release/libdevlib.a
else:win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../devlib/devlib/debug/libdevlib.a
else:win32:!win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../devlib/devlib/release/devlib.lib
else:win32:!win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../devlib/devlib/debug/devlib.lib
else:unix: PRE_TARGETDEPS += $$OUT_PWD/../devlib/devlib/libdevlib.a


include(../edge/edge_deps.pri)
