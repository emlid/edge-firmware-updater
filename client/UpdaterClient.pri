CONFIG += client

SOURCES += \
    $$PWD/Client.cpp \
    $$PWD/Updater.cpp

HEADERS += \
    $$PWD/Client.h \
    $$PWD/UpdaterConnection.h \
    $$PWD/Updater.h

SHARED_PATH = $$absolute_path(../main/shared)
include($${SHARED_PATH}/shared.pri)
INCLUDEPATH += $${SHARED_PATH}

include(impl/impl.pri)
