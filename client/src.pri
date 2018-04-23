CONFIG += client

SOURCES += \
  Client.cpp \
    $$PWD/Updater.cpp

HEADERS += \
  Client.h \
    $$PWD/UpdaterConnection.h \
    $$PWD/Updater.h

SHARED_PATH = ../main/shared
include($${SHARED_PATH}/shared.pri)
INCLUDEPATH += $${SHARED_PATH}

include(impl/impl.pri)
