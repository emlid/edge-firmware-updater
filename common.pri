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
