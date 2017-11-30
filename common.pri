SOURCES += $$PWD/src/main.cpp \
    $$PWD/src/FirmwareUpgraderWatcher.cpp \
    $$PWD/src/components/Flasher.cpp \
    $$PWD/src/components/ChecksumCalculator.cc \
    $$PWD/src/RpiBootSubtask.cpp \
    $$PWD/src/DeviceScannerSubtask.cpp \
    $$PWD/src/FlasherSubtask.cpp \
    $$PWD/src/SubtaskManager.cpp \
    $$PWD/src/ChecksumSubtask.cpp

HEADERS += \
    $$PWD/src/FirmwareUpgraderWatcher.h \
    $$PWD/src/components/Flasher.h \
    $$PWD/src/components/ChecksumCalculator.h \
    $$PWD/src/shared/States.h \
    $$PWD/src/AbstractSubtask.h \
    $$PWD/src/RpiBootSubtask.h \
    $$PWD/src/DeviceScannerSubtask.h \
    $$PWD/src/FlasherSubtask.h \
    $$PWD/src/SubtaskManager.h \
    $$PWD/src/ChecksumSubtask.h

REPC_SOURCE = $$PWD/src/shared/FirmwareUpgraderWatcher.rep

include(src/devapi/devapi.pri)
