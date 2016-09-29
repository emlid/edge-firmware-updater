QT += core
QT -= gui
QT += serialport

TARGET = edge_fw_updt
QT += widgets

FORMS += \
    mainwindow.ui

QMAKE_CFLAGS = -fpermissive
QMAKE_CXXFLAGS = -fpermissive
QMAKE_LFLAGS = -fpermissive

TEMPLATE = app

SOURCES += main.cpp \
    rpiboot.cpp \
    mainwindow.cpp \
    firmwareupgradecontroller.cpp \
    devicesearcher.cpp \
    storagedevice.cpp \
    storagedeviceflasher.cpp


HEADERS += \
    rpiboot.h \
    flashing_parameters.h \
    mainwindow.h \
    firmwareupgradecontroller.h \
    storagedevice.h \
    storagedeviceflasher.h



win32{
    SOURCES += dd_win.cpp \
        handledevice_win.cpp
    HEADERS += dd_win.h \
        handledevice_win.h
}

unix{
    SOURCES += dd.cpp \
        handledevice_linux.cpp
    HEADERS += dd.h \
        handledevice_linux.h

    LIBS += -L/usr/lib/ -ludev
}




LIBS += -lusb-1.0

LIBS += -L/usr/local/lib/ -lusb-1.0


