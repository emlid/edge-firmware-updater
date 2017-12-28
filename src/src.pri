SOURCES += \
    $$PWD/main.cpp \
    $$PWD/components/Flasher.cpp \
    $$PWD/components/ChecksumCalculator.cc \
    $$PWD/FlasherSubtask.cpp \
    $$PWD/SubtaskManager.cpp \
    $$PWD/ChecksumSubtask.cpp \
    $$PWD/DeviceInitializerSubtask.cpp \
    $$PWD/edge.cpp \
    $$PWD/EdgeFirmwareUpdater.cpp

HEADERS += \
    $$PWD/components/Flasher.h \
    $$PWD/components/ChecksumCalculator.h \
    $$PWD/components/Cancellable.h \
    $$PWD/AbstractSubtask.h \
    $$PWD/FlasherSubtask.h \
    $$PWD/ChecksumSubtask.h \
    $$PWD/DeviceInitializerSubtask.h \
    $$PWD/SubtaskManager.h \
    $$PWD/FlasherDataOwner.h \
    $$PWD/EdgeFirmwareUpdater.h \
    $$PWD/edge.h \

REPC_SOURCE = $$PWD/shared/EdgeFirmwareUpdaterIPC.rep

include(devlib/devlib.pri)
include(util/util.pri)
include(usb/usb.pri)
