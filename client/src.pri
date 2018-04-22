CONFIG += client

SOURCES += \
  Client.cpp \
    $$PWD/Updater.cpp

HEADERS += \
  Client.h \
    $$PWD/UpdaterConnection.h \
    $$PWD/Updater.h

include(../main/shared/shared.pri)
include(impl/impl.pri)
