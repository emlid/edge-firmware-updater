CONFIG += client

INCLUDEPATH += $${PWD}

SOURCES += \
    $$PWD/Client.cpp \
    $$PWD/Updater.cpp

HEADERS += \
    $$PWD/Client.h \
    $$PWD/UpdaterConnection.h \
    $$PWD/Updater.h

include(impl/impl.pri)
