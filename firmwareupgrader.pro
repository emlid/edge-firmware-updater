lessThan(QT_MAJOR_VERSION, 5) {
        error("Required at least Qt 5.9.")
}

equals(QT_MAJOR_VERSION, 5) : lessThan(QT_MINOR_VERSION, 9) {
        error("Required at least Qt 5.9.")
}

message(Qt version: $$[QT_VERSION])

QT += core testlib remoteobjects
QT -= gui


CONFIG += c++14

TARGET = fwupgrader
CONFIG += console
CONFIG -= app_bundle

TEMPLATE = app

OBJECTS_DIR = obj
MOC_DIR = moc
RCC_DIR = resources

SOURCES += $$PWD/src/main.cpp \
    $$PWD/src/FirmwareUpgrader.cpp \
    $$PWD/src/FirmwareUpgraderWatcher.cpp \
    $$PWD/src/Flasher.cpp \
    $$PWD/src/ChecksumCalculator.cc

HEADERS += \
    $$PWD/src/FirmwareUpgrader.h \
    $$PWD/src/FirmwareUpgraderWatcher.h \
    $$PWD/src/Flasher.h \
    $$PWD/src/ChecksumCalculator.h \
    $$PWD/src/shared/States.h

REPC_SOURCE = $$PWD/src/shared/FirmwareUpgraderWatcher.rep

include(src/devapi/devapi.pri)


# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0
