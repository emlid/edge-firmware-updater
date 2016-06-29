QT += core
QT -= gui
QT += serialport

TARGET = edge_fw_update
CONFIG += console
CONFIG -= app_bundle


QMAKE_CFLAGS = -fpermissive
QMAKE_CXXFLAGS = -fpermissive
QMAKE_LFLAGS = -fpermissive

TEMPLATE = app

SOURCES += main.cpp \
    dd.cpp \
    rpiboot.cpp

LIBS += -lusb-1.0

LIBS += -L/usr/local/lib/ -lusb-1.0
LIBS += -L/usr/lib/ -ludev

HEADERS += \
    dd.h \
    rpiboot.h \
    flashing_parameters.h
